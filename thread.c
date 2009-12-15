
#include <assert.h>

#include "thread.h"

static void
thread_simulate_rabbit(ThreadData* data, Map* map, Coord coord, Position* pos, int ger)
{
  Rabbit* rabbit = (Rabbit*)pos->obj;
  int i, total_free = ADJACENT;
  Coord tmp_coord;
  Position* tmp_pos = NULL;

  for(i = 0; i < ADJACENT; ++i) {
    tmp_coord = coord_at_direction(coord, i);
    
    if(!map_inside_coord(map, tmp_coord)) {
      data->free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    // verificar se existe uma raposa ou rocha
    tmp_pos = data->positions[i] = map_position_at_coord(map, tmp_coord);
    
    if(tmp_pos->is_rock || tmp_pos->obj)
    {
      data->free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    data->free_pos[i] = TRUE;
  }
  
  rabbit->last_procreation++;
  
  if(total_free == 0) {
    // não há casas livres, manter...
    position_move_rabbit(pos, rabbit);
    return;
  }
  
  // seleccionar casa a mover
  int index = (ger + Coord_x(coord) + Coord_y(coord)) % total_free;
  int real_index = -1;
  
  for(i = 0; i < ADJACENT; ++i) {
    if(data->free_pos[i]) {
      if(!index--) {
        real_index = i;
        break;
      }
    }
  }
  
  if(rabbit->last_procreation >= map->ger_proc_coelhos) {
    // vai reproduzir-se...
    rabbit->last_procreation = 0;
    position_move_rabbit(pos, object_new_rabbit());
  }
  
  position_move_rabbit(data->positions[real_index], rabbit);
}

static void
thread_simulate_fox(ThreadData* data, Map* map, Coord coord, Position* pos, int ger)
{
  Fox* fox = (Fox*)pos->obj;
  int i, total_free = ADJACENT, rabbits = 0;
  Coord tmp_coord;
  Position* tmp_pos;
  
  for(i = 0; i < ADJACENT; ++i) {
    tmp_coord = coord_at_direction(coord, i);
    
    if(!map_inside_coord(map, tmp_coord)) {
      data->free_pos[i] = FALSE;
      --total_free;
      data->with_rabbits[i] = FALSE;
      continue;
    }
    
    tmp_pos = data->positions[i] = map_position_at_coord(map, tmp_coord);
    
    if(tmp_pos->is_rock ||
      (tmp_pos->obj && tmp_pos->obj->type == FOX))
    {
      data->free_pos[i] = FALSE;
      --total_free;
      data->with_rabbits[i] = FALSE;
      continue;
    }
    
    if(tmp_pos->obj && tmp_pos->obj->type == RABBIT) {
      data->with_rabbits[i] = TRUE;
      data->free_pos[i] = TRUE;
      ++rabbits;
      continue;
    }
    
    data->free_pos[i] = TRUE;
    data->with_rabbits[i] = FALSE;
  }
  
  fox->last_food++;
  fox->last_procreation++;
  
  if(total_free == 0) {
    // não há casas livres, manter...
    position_move_fox(pos, fox);
    return;
  }
 
  int real_index = -1, index = ger + Coord_x(coord) + Coord_y(coord);
  Boolean *bool_array = NULL;

  if(rabbits > 0) {
    index %= rabbits;
    bool_array = data->with_rabbits;
  } else {
    bool_array = data->free_pos;
    index %= total_free;
  }

  for(i = 0; i < ADJACENT; ++i) {
    if(bool_array[i]) {
      if(!index--) {
        real_index = i;
        break;
      }
    }
  }

  if(fox->last_procreation >= map->ger_proc_raposas) {
    // vai procriar
    position_move_fox(pos, object_new_fox());
    fox->last_procreation = 0;
  }

  position_move_fox(data->positions[real_index], fox);
}

void
thread_simulate_position(ThreadData* data, Map* map, Position* pos, Coord coord, int ger)
{
  if(pos->obj) {
    if(pos->obj->type == RABBIT)
      thread_simulate_rabbit(data, map, coord, pos, ger);
    if(pos->obj->type == FOX)
      thread_simulate_fox(data, map, coord, pos, ger);
  }
}

void
thread_resolve_conflict(Map* map, Position* pos)
{
  ObjectList* list = pos->start;
  Rabbit* best_rabbit = NULL;
  Fox* oldest_fox = NULL;
  Fox* hungriest_fox = NULL;
  Object* obj;
  Fox* fox;
  Rabbit* rabbit;
  
  while(list) {
    obj = list->obj;
    
    if(obj->type == FOX) {
      fox = (Fox*)obj;
      
      if(!hungriest_fox ||
        hungriest_fox->last_food < fox->last_food)
      {
        hungriest_fox = fox;
      }
      
      if(!oldest_fox ||
        oldest_fox->last_procreation < fox->last_procreation)
      {
        oldest_fox = fox;
      }

    } else if(obj->type == RABBIT) {
      rabbit = (Rabbit*)obj;
      
      if(!best_rabbit ||
        best_rabbit->last_procreation < rabbit->last_procreation)
      {
        best_rabbit = rabbit;
      }
    }
    
    list = list->next;
  }
  
  Boolean fox_eat = FALSE;
  
  if(best_rabbit && !oldest_fox) { // so coelhos
    pos->obj = (Object*)best_rabbit;
  } else if(best_rabbit && oldest_fox) { // coelhos e raposas
    pos->obj = (Object*)oldest_fox;
    fox_eat = TRUE;
  } else if(hungriest_fox) { // raposas
    pos->obj = (Object*)hungriest_fox;
  } else {
    pos->obj = NULL;
  }
  
  // libertar lista
  list = pos->start;
  ObjectList* tmp;
  
  while(list) {
    if(pos->obj != list->obj)
      free(list->obj);
    
    tmp = list;
    list = list->next;
    free(tmp);
  }
  
  pos->start = NULL;
  pos->end = NULL;
  
  if(pos->obj && pos->obj->type == FOX) {
    fox = (Fox*)pos->obj;
        
    if(fox_eat)
      fox->last_food = 0;
        
    if(fox->last_food >= map->ger_alim_raposas) {
      // morreu
      free(pos->obj);
      pos->obj = NULL;
    }
  }
}
