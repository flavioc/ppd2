
#include <stdlib.h>
#include <assert.h>

#include "position.h"
#include "map.h"

void
position_init(Position* pos)
{
  pos->is_rock = FALSE;
  pos->obj = NULL;
  pos->start = NULL;
  pos->end = NULL;
  
  pthread_mutex_init(&pos->mutex, NULL);
}

void
position_free(Position* pos)
{
  if(pos->obj)
    free(pos->obj);
  
  pthread_mutex_destroy(&pos->mutex);
}

static inline void
position_add_list(Position* pos, Object* obj)
{
  pthread_mutex_lock(&pos->mutex);
  
  ObjectList* new = object_list_new(obj, NULL);
  
  if(pos->start) {
    pos->end->next = new;
  } else {
    pos->start = new;
  }
  pos->end = new;
  
  pthread_mutex_unlock(&pos->mutex);
}

void
position_move_fox(Position* pos, Fox* fox)
{
  position_add_list(pos, (Object*)fox);
}

void
position_move_rabbit(Position* pos, Rabbit* rabbit)
{
  position_add_list(pos, (Object*)rabbit);
}