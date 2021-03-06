
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "map.h"
#include "thread.h"

#define MASTER_THREAD 0
#define THREAD_PARAM void*

static int NTHREADS;
static ThreadData* thread_data;
static Map* map = NULL;

static int LINES_PER_THREAD_FIRST_PASS;
static int LINES_PER_THREAD_SECOND_PASS;

#define MAIN_LOCK pthread_mutex_lock(&map->mutex)
#define MAIN_UNLOCK pthread_mutex_unlock(&map->mutex)

static inline void
update_second_pass(void)
{
  Boolean cond;
  int row, i;
  int total = min(LINES_PER_THREAD_SECOND_PASS + 1, map->lin - map->next_row_second_pass);
  
  for(i = 0, row = map->next_row_second_pass; i < total; ++row, ++i) {
    cond = map->generation_second_pass == map->rows_ger_first_pass[row] &&
      map->rows_ger_second_pass[row] == map->generation_second_pass-1;
      
    if(!cond) {
      map->proceed_second_pass = FALSE;
      return;
    }
  }
  
  map->proceed_second_pass = TRUE;
  pthread_cond_signal(&map->cond);
}

static inline void
update_first_pass(void)
{
  Boolean cond;
  int total = min(LINES_PER_THREAD_FIRST_PASS + 1, map->lin - map->next_row_first_pass);
  int i, row;
  
  map->proceed_first_pass = TRUE;
  
  for(i = 0, row = map->next_row_first_pass; i < total; ++i, ++row) {
    cond = map->generation_first_pass-1 == map->rows_ger_first_pass[row]
      && map->rows_ger_first_pass[row] == map->rows_ger_second_pass[row];
    
    if(!cond) {
      map->proceed_first_pass = FALSE;
      return;
    }
  }
  
  map->proceed_first_pass = TRUE;
  pthread_cond_signal(&map->cond);
}

static THREAD_PARAM
run_thread(THREAD_PARAM arg)
{
  int thread_num = (int)arg;
  int row;
  Coord coord;
  int i, j;
  ThreadData *data = thread_data + thread_num;
  int total_lines, pass_line, pass_generation;
  
  while(TRUE) {
    
    MAIN_LOCK;
    
    while(!map->proceed_first_pass &&
          !map->proceed_second_pass &&
          !map->the_end)
      pthread_cond_wait(&map->cond, &map->mutex);
    
    if(map->the_end) {
      MAIN_UNLOCK;
      break;
    }
    
    /*printf("{%d, %d} %d - %d\n",
      map->generation_first_pass,
      map->generation_second_pass,
      map->next_row_first_pass,
      map->next_row_second_pass);
    */
    if(map->proceed_second_pass) {
      // take work from second pass
      total_lines = min(LINES_PER_THREAD_SECOND_PASS, map->lin - map->next_row_second_pass);
      pass_line = map->next_row_second_pass;
      pass_generation = map->generation_second_pass;
      map->next_row_second_pass = (map->next_row_second_pass + total_lines) % map->lin;
      
      if(map->next_row_second_pass == 0) {
        map->generation_second_pass++;
        if(map->generation_second_pass == map->n_ger + 1)
          map->more_second_pass = FALSE;
      }
      
      if(map->more_second_pass)
        update_second_pass();
      else
        map->proceed_second_pass = FALSE;
      
      MAIN_UNLOCK;
      
      // do second pass work
      //printf("%d: Second [%d, %d]\n", pass_generation, pass_line, pass_line + total_lines);

      for(i = 0, row = pass_line; i < total_lines; ++i, ++row) {
        for(j = 0; j < map->col; ++j) {
          thread_resolve_conflict(map, map_position_at(map, row, j));
        }
        map->rows_ger_second_pass[row] = pass_generation;
      }
      //printf("%d: end Second [%d, %d]\n", pass_generation, pass_line, pass_line + total_lines);

      if(map->the_end)
        break;

      if(pass_generation == map->n_ger &&
        pass_line + total_lines == map->lin)
      {
        map->the_end = TRUE;
        pthread_cond_broadcast(&map->cond);
        break;
      }

      if(!map->more_first_pass)
        continue;

      if(!map->proceed_first_pass) {
        MAIN_LOCK;
        if(!map->proceed_first_pass)
          update_first_pass();
        MAIN_UNLOCK;
      }
      
    } else { // proceed first pass

      // take work from first pass
      pass_line = map->next_row_first_pass;
      total_lines = min(LINES_PER_THREAD_FIRST_PASS, map->lin - map->next_row_first_pass);
      pass_generation = map->generation_first_pass;
      map->next_row_first_pass = (map->next_row_first_pass + total_lines) % map->lin;
      
      if(map->next_row_first_pass == 0) {
        map->generation_first_pass++;
        if(map->generation_first_pass == map->n_ger + 1)
          map->more_first_pass = FALSE;
      }
      
      if(map->more_first_pass)
        update_first_pass();
      else
        map->proceed_first_pass = FALSE;
      
      MAIN_UNLOCK;
      
      //printf("%d: First [%d, %d]\n", pass_generation, pass_line, pass_line + total_lines);

      // do first pass work
      for(i = 0, row = pass_line; i < total_lines; ++i, ++row) {
        Coord_x(coord) = row;

        for(j = 0; j < map->col; ++j) {
          Coord_y(coord) = j;

          thread_simulate_position(data, map, map_position_at(map, row, j), coord, pass_generation - 1);
        }
        
        map->rows_ger_first_pass[row] = pass_generation;
      }
      //printf("%d: end First [%d, %d]\n", pass_generation, pass_line, pass_line + total_lines);

      if(map->the_end)
        break;

      if(!map->more_second_pass)
        continue;

      if(!map->proceed_second_pass) {
        MAIN_LOCK;
        if(!map->proceed_second_pass)
          update_second_pass();
        MAIN_UNLOCK;
      }
    }
  }
  
  if(thread_num != MASTER_THREAD)
    pthread_exit(NULL);
    
  return NULL;
}

static void
simulate(void)
{
  // create thread data
  thread_data = (ThreadData*)malloc(sizeof(ThreadData) * NTHREADS);
  
  int i;
  pthread_t tids[NTHREADS];
  
  // schedule lines per task
  if(NTHREADS == 1) {
    LINES_PER_THREAD_FIRST_PASS = map->lin;
    LINES_PER_THREAD_SECOND_PASS = map->lin;
  } else {
    int f = 2.0;
    int blocks = max(1, (int)((double)map->lin / (f * (double)NTHREADS)));
    
    LINES_PER_THREAD_FIRST_PASS = blocks;
    LINES_PER_THREAD_SECOND_PASS = blocks;
    
    printf("%d\n", blocks);
  }
  
  for(i = 1; i < NTHREADS; ++i) {
    pthread_create(&tids[i], NULL, run_thread, (void*)i);  
  }
  
  run_thread((THREAD_PARAM)MASTER_THREAD);
  
  for(i = 1; i < NTHREADS; ++i) {
    pthread_join(tids[i], NULL);
  }
  
  free(thread_data);
}

int
main(int argc, char** argv)
{
  if(argc != 2) {
    fprintf(stderr, "usage: simulator <n-threads>\n");
    return EXIT_FAILURE;
  }
  
  NTHREADS = atoi(argv[1]);
  
  map = map_read(stdin);
  
  if(!map) {
    fprintf(stderr, "Failed to read map\n");
    exit(EXIT_FAILURE);
  }
  
  //map_print(map, stdout);
  
  simulate();
  
  //map_print(map, stdout);
  map_output(map, stdout);
  
  map_free(map);
  
  return EXIT_SUCCESS;
}
