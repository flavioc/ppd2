#ifndef THREAD_H
#define THREAD_H

#include "config.h"

#include "utils.h"
#include "position.h"

typedef struct {
  Boolean free_pos[ADJACENT];
  Boolean with_rabbits[ADJACENT];
  Position* positions[ADJACENT];
  
  int first_pass_lines[LINES_PER_THREAD_FIRST_PASS];
  int second_pass_lines[LINES_PER_THREAD_SECOND_PASS];
} ThreadData;

#endif