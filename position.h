#ifndef POSITION_H
#define POSITION_H

#include <stdlib.h>

#include "utils.h"
#include "object.h"

typedef struct {
  Boolean is_rock;
  Object* obj;
  
  Rabbit* best_rabbit;
  Fox* hungriest_fox;
  Fox* oldest_fox;
  
  Object* free_objects[4];
  int current_free;
} Position;

void position_init(Position* pos);
void position_add_free(Position* pos, Object* obj);
int  position_clean_free(Position* pos, Object* except);

#endif