
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
static int BLOCO;
static int* count1;
static int* count2;

static THREAD_PARAM
run_thread(THREAD_PARAM arg)
{
  int thread_num = (int)arg;
  Boolean last_thread = (thread_num == NTHREADS - 1);
  ThreadData *data = thread_data + thread_num;
  int i, j;
  int bloco = BLOCO, start = BLOCO * thread_num, end;
  Position* pos;
  Coord coord;
  Boolean espera = FALSE;
  int ger = 0;
  
  if(last_thread) {
    bloco = map->lin - start;
  }
  end = start + bloco;
  
  while(TRUE) { 
    for(i = start; i < end; ++i) {
      Coord_x(coord) = i;
      for(j = 0; j < map->col; ++j) {
        Coord_y(coord) = j;
        pos = map_position_at(map, i, j);
        thread_simulate_position(data, map, pos, coord, ger);
      }
    }

    pthread_mutex_lock(&map->mutex);
    --count1[ger];
    espera = TRUE;
    if(!count1[ger]) {
      pthread_cond_broadcast(&map->cond);
      espera = FALSE;
    }
    pthread_mutex_unlock(&map->mutex);

    if(espera) {
      pthread_mutex_lock(&map->mutex);
      while(count1[ger])
        pthread_cond_wait(&map->cond, &map->mutex);
      pthread_mutex_unlock(&map->mutex);
    } 
    
    //printf("%d: passo 2 ger %d [%d, %d]\n", thread_num, ger, start, end);

    for(i = start; i < end; ++i) {
      Coord_x(coord) = i;

      for(j = 0; j < map->col; ++j) {
        pos = map_position_at(map, i, j);
        thread_resolve_conflict(map, pos);
      }
    }

    pthread_mutex_lock(&map->mutex);
    --count2[ger];
    espera = TRUE;
    if(!count2[ger]) {
      //map_print(map, stdout);
      pthread_cond_broadcast(&map->cond);
      espera = FALSE;
    }
    pthread_mutex_unlock(&map->mutex);

    if(espera) {
      pthread_mutex_lock(&map->mutex);
      while(count2[ger])
        pthread_cond_wait(&map->cond, &map->mutex);
      pthread_mutex_unlock(&map->mutex);
    }
  
    ++ger;
    
    if(ger == map->n_ger)
      break;
      
    //printf("%d: passo 1 ger %d\n", thread_num, ger);
  }
  
  return NULL;
}

static void
simulate(void)
{
  // create thread data
  thread_data = (ThreadData*)malloc(sizeof(ThreadData) * NTHREADS);
  
  int i;
  pthread_t tids[NTHREADS];
  
  count1 = (int*)malloc(sizeof(int)*map->n_ger);
  count2 = (int*)malloc(sizeof(int)*map->n_ger);
  
  for(i = 0; i < map->n_ger; ++i) {
    count1[i] = NTHREADS;
    count2[i] = NTHREADS;
  }
  
  // schedule
  if(NTHREADS == 1) {
    BLOCO = map->lin;
  } else {
    BLOCO = map->lin / NTHREADS;
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
main(int argc, char **argv)
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