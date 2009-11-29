#ifndef CONFIG_H
#define CONFIG_H

#define MULTITHREAD 1

#ifdef MULTITHREAD
#include "pthread.h"

#define LINE_PER_THREAD 2
#endif

#endif