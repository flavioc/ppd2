
#include <stdlib.h>

#include "position.h"

void
position_add_object(Position *pos, Object* obj)
{
  if(pos->list_start) {
    pos->list_end = object_add_list(pos->list_end, obj);
  } else {
    pos->list_start = pos->list_end = object_new_list(obj);
  }
}

void
position_free_list(Position* pos)
{
  ObjectList* list = pos->list_start;
  ObjectList* next = NULL;
  
  while(list) {
    next = ObjectList_next(list);
    free(list);
    list = next;
  }
  
  pos->list_start = NULL;
  pos->list_end = NULL;
}