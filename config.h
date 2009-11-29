#ifndef CONFIG_H
#define CONFIG_H

#define MULTITHREAD 1

#ifdef MULTITHREAD
#include "pthread.h"

#define LINES_PER_THREAD_FIRST_PASS 2
#define LINES_PER_THREAD_SECOND_PASS 2
#endif

#endif