
#include <stdlib.h>

#include "map.h"
#include "thread.h"

#define MASTER_THREAD 0
#define THREAD_PARAM void*

static int nthreads = 4;
static ThreadData* thread_data;

static THREAD_PARAM
run_thread(THREAD_PARAM data)
{
  int thread_num = (int)data;
  ThreadData *mydata = thread_data + thread_num;
  
  printf("Sou a thread %d\n", thread_num);
  
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
