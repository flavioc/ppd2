
#include <stdlib.h>
#include <assert.h>

#include "position.h"
#include "map.h"

void
position_init(Position* pos)
{
  pos->is_rock = FALSE;
  pos->obj = NULL;
  pos->best_rabbit = NULL;
  pos->hungriest_fox = NULL;
  pos->oldest_fox = NULL;
  pos->current_free = 0;
  pthread_mutex_init(&pos->mutex, NULL);
}

void
position_free(Position* pos)
{
  if(pos->obj)
    free(pos->obj);
  
  pthread_mutex_destroy(&pos->mutex);
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

void
position_move_fox(Position* pos, Fox* fox)
{
  pthread_mutex_lock(&pos->mutex);

  Boolean had_rabbit = FALSE;
  
  if(pos->best_rabbit) {
    had_rabbit = TRUE;
    
    free(pos->best_rabbit); // libertar coelho
    map_rabbit_death_inc(map, 1);
    pos->best_rabbit = NULL;
    
    if(pos->hungriest_fox) {
      position_add_free(pos, (Object*)pos->hungriest_fox);
      pos->hungriest_fox = NULL;
    }
  } else if(pos->oldest_fox && !pos->hungriest_fox)
    had_rabbit = TRUE;
  
  if(!had_rabbit) {
    if(!pos->hungriest_fox ||
      pos->hungriest_fox->last_food < fox->last_food)
    {
      if(pos->hungriest_fox)
        position_add_free(pos, (Object*)pos->hungriest_fox);
      pos->hungriest_fox = fox;
    }
  }
  
  if(!pos->oldest_fox ||
    pos->oldest_fox->last_procreation < fox->last_procreation)
  {
    if(pos->oldest_fox)
      position_add_free(pos, (Object*)pos->oldest_fox);
    pos->oldest_fox = fox;
  }
  
  pthread_mutex_unlock(&pos->mutex);
}

void
position_move_rabbit(Position* pos, Rabbit* rabbit)
{
  pthread_mutex_lock(&pos->mutex);
  
  if(pos->oldest_fox) {
    if(pos->hungriest_fox) {
      position_add_free(pos, (Object*)pos->hungriest_fox);
      pos->hungriest_fox = NULL;
    }
    pos->best_rabbit = NULL;
    free(rabbit); // coelho não necessário
    map_rabbit_death_inc(map, 1);
  } else {
    Rabbit *tokill = NULL;
    
    if(!pos->best_rabbit || 
      pos->best_rabbit->last_procreation < rabbit->last_procreation)
    {
      if(pos->best_rabbit)
        tokill = pos->best_rabbit;
      pos->best_rabbit = rabbit;
    } else
      tokill = rabbit;
    if(tokill) {
      map_rabbit_death_inc(map, 1);
      free(tokill);
    }
  }

  pthread_mutex_unlock(&pos->mutex);
}