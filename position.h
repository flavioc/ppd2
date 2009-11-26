#ifndef POSITION_H
#define POSITION_H

#include <stdlib.h>

#include "utils.h"
#include "object.h"

typedef struct {
  Boolean is_rock;
  Object* obj;
  ObjectList* list_start;
  ObjectList* list_end;
} Position;

void position_add_object(Position *pos, Object* obj);
void position_free_list(Position* pos);

#define position_has_list(POS) ((POS)->list_start != NULL)

#endif