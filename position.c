
#include <stdlib.h>
#include <assert.h>

#include "position.h"

void
position_init(Position* pos)
{
  pos->is_rock = FALSE;
  pos->obj = NULL;
  pos->best_rabbit = NULL;
  pos->hungriest_fox = NULL;
  pos->oldest_fox = NULL;
  pos->current_free = 0;
}

void
position_add_free(Position* pos, Object* obj)
{
  int i;
  
  for(i = 0; i < pos->current_free; ++i) {
    if(pos->free_objects[i] == obj)
      return;
  }
  
  assert(i < 4);
  pos->free_objects[i] = obj;
  pos->current_free++;
}

int
position_clean_free(Position* pos, Object* except)
{
  if(pos->current_free > 0) {
    int i, total = 0;
  
    for(i = 0; i < pos->current_free; ++i) {
      if(pos->free_objects[i] != except) {
        free(pos->free_objects[i]);
        ++total;
      }
    }
  
    pos->current_free = 0;
    
    return total;
  }
  
  return 0;
}