#ifndef POSITION_H
#define POSITION_H

#include <stdlib.h>

#include "config.h"
#include "utils.h"
#include "object.h"

#define MAX_FREE_OBJS 20

typedef struct {
  Boolean is_rock;
  Object* obj;
  
  Rabbit* best_rabbit;
  Fox* hungriest_fox;
  Fox* oldest_fox;
  
  Object* free_objects[MAX_FREE_OBJS];
  int current_free;
  
  pthread_mutex_t mutex;
  pthread_mutex_t obj_mutex;
} Position;

void position_init(Position* pos);
void position_free(Position* pos);
void position_add_free(Position* pos, Object* obj);
void position_clean_free(Position* pos, Object* except);
void position_move_fox(Position* pos, Fox* fox);
void position_move_rabbit(Position* pos, Rabbit* rabbit);

#endif