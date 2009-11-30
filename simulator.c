
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "thread.h"

#define MASTER_THREAD 0
#define THREAD_PARAM void*

static int nthreads = 2;
static ThreadData* thread_data;

//#define DEBUG

static int LINES_PER_THREAD_FIRST_PASS = 5;
static int LINES_PER_THREAD_SECOND_PASS = 4;

#define MAIN_LOCK pthread_mutex_lock(&map->mutex)
#define MAIN_UNLOCK pthread_mutex_unlock(&map->mutex)

static inline void
update_second_pass()
{
  if(map->proceed_second_pass)
    return;
    
  int row;
  int total = map->next_row_second_pass +
    min(LINES_PER_THREAD_SECOND_PASS + 1, map->lin - map->next_row_second_pass);
  
  if(map->rows_ger_second_pass[map->next_row_second_pass] != map->generation_second_pass)
    map->proceed_second_pass = FALSE;
  else {
    map->proceed_second_pass = TRUE;
    
    for(row = map->next_row_second_pass; row < total; ++row) {
      if((map->rows_ger_second_pass[row] + 1) != map->rows_ger_first_pass[row]) {
        map->proceed_second_pass = FALSE;
        break;
      }
    }
  }
  
  if(map->proceed_second_pass) {
#ifdef DEBUG
    printf("CAN PROCEED SECOND PASS!\n");
#endif
    pthread_cond_signal(&map->cond);
#ifdef DEBUG
  } else {
      printf("NOT PROCEED SECOND PASS!\n");
#endif
  }
}

static inline void
update_first_pass()
{
  if(map->proceed_first_pass)
    return;
  
  int total = min(LINES_PER_THREAD_FIRST_PASS + 1, map->lin - map->next_row_first_pass);
  
  if(memcmp(map->rows_ger_first_pass + map->next_row_first_pass,
            map->rows_ger_second_pass + map->next_row_first_pass,
        sizeof(int) * total) == 0
     && map->generation_first_pass == map->rows_ger_first_pass[map->next_row_first_pass])
  {
    map->proceed_first_pass = TRUE;
#ifdef DEBUG
    printf("CAN PROCEED FIRST PASS!\n");
#endif
    pthread_cond_signal(&map->cond);
  } else {
#ifdef DEBUG
    printf("NOT PROCEED FIRST PASS\n");
#endif
    map->proceed_first_pass = FALSE;
  }
}

static THREAD_PARAM
run_thread(THREAD_PARAM arg)
{
  int thread_num = (int)arg;
  int row;
  Coord coord;
  int i, j;
  ThreadData *data = thread_data + thread_num;
  Boolean got_first_not_second;
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
    
    if(map->proceed_second_pass) {
      got_first_not_second = FALSE;
      
      total_lines = min(LINES_PER_THREAD_SECOND_PASS, map->lin - map->next_row_second_pass);
      pass_line = map->next_row_second_pass;
      pass_generation = map->rows_ger_second_pass[pass_line] + 1;
      map->next_row_second_pass = (map->next_row_second_pass + total_lines) % map->lin;
      
      if(map->next_row_second_pass == 0)
        map->generation_second_pass++;
      
      if(map->generation_second_pass == map->n_ger && map->next_row_second_pass == 0)
        map->more_second_pass = FALSE;
      
      map->proceed_second_pass = FALSE;
      update_second_pass();
      
      MAIN_UNLOCK;
      
    } else { // proceed first pass
      got_first_not_second = TRUE;
      
      pass_line = map->next_row_first_pass;
      total_lines = min(LINES_PER_THREAD_FIRST_PASS, map->lin - map->next_row_first_pass);
      pass_generation = map->rows_ger_first_pass[pass_line] + 1;
      map->next_row_first_pass = (map->next_row_first_pass + total_lines) % map->lin;
      
      if(map->next_row_first_pass == 0)
        map->generation_first_pass++;
      
      map->proceed_first_pass = FALSE;
      
      if(map->generation_first_pass == map->n_ger && map->next_row_first_pass == 0)
        map->more_first_pass = FALSE;
      else
        update_first_pass();
      
      MAIN_UNLOCK;
    }
    
    if(got_first_not_second) {
#ifdef DEBUG
      printf("%d: first pass %d to %d\n", thread_num, pass_line, pass_line + total_lines - 1);
#endif

      for(i = 0, row = pass_line; i < total_lines; ++i, ++row) {
        Coord_x(coord) = row;
        
        for(j = 0; j < map->col; ++j) {
          Coord_y(coord) = j;
          
          thread_simulate_position(data, map_position_at(map, row, j), coord, pass_generation-1);
        }
        
        map->rows_ger_first_pass[row] = pass_generation;
      }
      
#ifdef DEBUG
      printf("%d: done first pass %d to %d\n", thread_num, pass_line, pass_line + total_lines - 1);
#endif

      if(map->the_end)
        break;
        
      if(!map->more_second_pass)
        continue;
      
      MAIN_LOCK;
      update_second_pass();
      MAIN_UNLOCK;
      
    } else { // !got_first_not_second
#ifdef DEBUG
      printf("%d: second pass %d to %d\n", thread_num, pass_line, pass_line + total_lines - 1);
#endif
      
      for(i = 0, row = pass_line; i < total_lines; ++i, ++row) {
        for(j = 0; j < map->col; ++j)
          thread_resolve_conflict(map_position_at(map, row, j));
        
        map->rows_ger_second_pass[row] = pass_generation;
      }
      
#ifdef DEBUG
      printf("%d: done second pass %d to %d\n", thread_num, pass_line, pass_line + total_lines - 1);
#endif
      
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
      
      MAIN_LOCK;
      update_first_pass();
      MAIN_UNLOCK;
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
  thread_data = (ThreadData*)malloc(sizeof(ThreadData) * nthreads);
  
  // create worker threads
  int i;
  pthread_t tids[nthreads];
  
  for(i = 1; i < nthreads; ++i) {
    pthread_create(&tids[i], NULL, run_thread, (void*)i);  
  }
  
  run_thread((THREAD_PARAM)MASTER_THREAD);
  
  for(i = 1; i < nthreads; ++i) {
    pthread_join(tids[i], NULL);
  }
  
  free(thread_data);
}

int
main()
{
  map = map_read(stdin);
  
  if(!map) {
    fprintf(stderr, "Failed to read map\n");
    exit(EXIT_FAILURE);
  }
  
  map_print(map);
  
  simulate();
  
  map_print(map);
  
  map_free(map);
  
  return EXIT_SUCCESS;
}
