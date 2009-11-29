#ifndef THREAD_H
#define THREAD_H

#include "config.h"

#include "utils.h"
#include "position.h"

typedef struct {
  Boolean free_pos[ADJACENT];
  Boolean with_rabbits[ADJACENT];
  Position* positions[ADJACENT];
} ThreadData;

#endif