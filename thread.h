#ifndef THREAD_H
#define THREAD_H

#include "config.h"

#include "utils.h"
#include "position.h"
#include "map.h"

typedef struct {
  Boolean free_pos[ADJACENT];
  Boolean with_rabbits[ADJACENT];
  Position* positions[ADJACENT];
} ThreadData;

void thread_simulate_position(ThreadData* data, Map* map, Position* pos, Coord coord, int ger);
void thread_resolve_conflict(Position* pos);

#endif