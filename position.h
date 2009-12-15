#ifndef POSITION_H
#define POSITION_H

#include <stdlib.h>

#include "config.h"
#include "utils.h"
#include "object.h"

typedef struct {
  Boolean is_rock;
  Object* obj;
  
  ObjectList* start;
  ObjectList* end;
  
  pthread_mutex_t mutex;
} Position;

void position_init(Position* pos);
void position_free(Position* pos);
void position_add_free(Position* pos, Object* obj);
void position_clean_free(Position* pos, Object* except);
void position_move_fox(Position* pos, Fox* fox);
void position_move_rabbit(Position* pos, Rabbit* rabbit);

#endif