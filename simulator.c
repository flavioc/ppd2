
#include <stdlib.h>

#include "map.h"
#include "thread.h"

#define MASTER_THREAD 0
#define THREAD_PARAM void*

static int nthreads = 4;
static ThreadData* thread_data;

#define MAIN_LOCK pthread_mutex_lock(&map->mutex)
#define MAIN_UNLOCK pthread_mutex_unlock(&map->mutex)

static THREAD_PARAM
run_thread(THREAD_PARAM arg)
{
  int thread_num = (int)arg;
  int i;
  ThreadData *data = thread_data + thread_num;
  
  printf("Sou a thread %d\n", thread_num);
  
  while(TRUE) {
    MAIN_LOCK;
    
    while(!map->proceed_first_pass &&
          !map->proceed_second_pass &&
          !map->the_end)
    {
      pthread_cond_wait(&map->cond, &map->mutex);
    }
    
    if(map->the_end) {
      MAIN_UNLOCK;
      break;
    }
    
    if(map->proceed_second_pass) {
      // copy lines to work!
      for(i = 0; i < LINES_PER_THREAD_SECOND_PASS; ++i) {
        data->second_pass_lines[i] = map->next_row_second_pass;
        map->next_row_second_pass = map_next_row(map, map->next_row_second_pass);
      }
      
    } else { // proceed first pass
      // copy lines to work!
      for(i = 0; i < LINES_PER_THREAD_FIRST_PASS; ++i) {
        data->first_pass_lines[i] = map->next_row_first_pass;
        map->next_row_first_pass = map_next_row(map, map->next_row_first_pass);
      }
      printf("first pass: %d\n", map->next_row_first_pass);
      
      // verify if it's possible to proceed to the next set of rows
    }
    
    MAIN_UNLOCK;
    
    break;
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
  
  map_print(map);
  
  simulate();
  
  map_free(map);
  
  return EXIT_SUCCESS;
}
