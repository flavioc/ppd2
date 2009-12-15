
#include <assert.h>

#include "thread.h"

static void
thread_simulate_rabbit(ThreadData* data, Map* map, Coord coord, Position* pos, int ger)
{
  Rabbit* rabbit = (Rabbit*)pos->obj;
  int i, total_free = ADJACENT;
  Coord tmp_coord;
  Position* tmp_pos = NULL;
  Boolean free_pos[ADJACENT];
  Position* positions[ADJACENT];
  
  for(i = 0; i < ADJACENT; ++i) {
    positions[i] = NULL;
    free_pos[i] = FALSE;
  }

  for(i = 0; i < ADJACENT; ++i) {
    tmp_coord = coord_at_direction(coord, i);
    
    if(!map_inside_coord(map, tmp_coord)) {
      free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    // verificar se existe uma raposa ou rocha
    tmp_pos = positions[i] = map_position_at_coord(map, tmp_coord);
    
    if(tmp_pos->is_rock || tmp_pos->obj)
    {
      free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    free_pos[i] = TRUE;
  }
  
  if(total_free == 0) {
    // não há casas livres, manter...
    position_move_rabbit(pos, rabbit);
    return;
  }
  
  // seleccionar casa a mover
  int index = (ger + Coord_x(coord) + Coord_y(coord)) % total_free;
  int real_index = -1;
  
  for(i = 0; i < ADJACENT; ++i) {
    if(free_pos[i]) {
      if(!index--) {
        real_index = i;
        break;
      }
    }
  }
  
  if(rabbit->last_procreation >= map->ger_proc_coelhos) {
    // vai reproduzir-se...
    rabbit->procreate = TRUE;
    position_move_rabbit(pos, object_new_rabbit());
  }
  
  position_move_rabbit(positions[real_index], rabbit);
}

static void
thread_simulate_fox(ThreadData* data, Map* map, Coord coord, Position* pos, int ger)
{
  Fox* fox = (Fox*)pos->obj;
  Boolean free_pos[ADJACENT], with_rabbits[ADJACENT];
  int i, total_free = ADJACENT, rabbits = 0;
  Coord tmp_coord;
  Position* tmp_pos;
  Position* positions[ADJACENT];
  
  for(i = 0; i < ADJACENT; ++i) {
    positions[i] = NULL;
    free_pos[i] = FALSE;
    with_rabbits[i] = FALSE;
  }
  
  for(i = 0; i < ADJACENT; ++i) {
    tmp_coord = coord_at_direction(coord, i);
    
    if(!map_inside_coord(map, tmp_coord)) {
      free_pos[i] = FALSE;
      --total_free;
      with_rabbits[i] = FALSE;
      continue;
    }
    
    tmp_pos = positions[i] = map_position_at_coord(map, tmp_coord);
    
    if(tmp_pos->is_rock ||
      (tmp_pos->obj && tmp_pos->obj->type == FOX))
    {
      free_pos[i] = FALSE;
      --total_free;
      with_rabbits[i] = FALSE;
      continue;
    }
    
    if(tmp_pos->obj && tmp_pos->obj->type == RABBIT) {
      with_rabbits[i] = TRUE;
      free_pos[i] = TRUE;
      ++rabbits;
      continue;
    }
    
    free_pos[i] = TRUE;
    with_rabbits[i] = FALSE;
  }
  
  if(total_free == 0) {
    // não há casas livres, manter...
    position_move_fox(pos, fox);
    return;
  }
 
  int real_index = -1, index = ger + Coord_x(coord) + Coord_y(coord);
  Boolean *bool_array = NULL;

  if(rabbits > 0) {
    index %= rabbits;
    bool_array = with_rabbits;
  } else {
    bool_array = free_pos;
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
    fox->procreate = TRUE;
    position_move_fox(pos, object_new_fox());
  }

  position_move_fox(positions[real_index], fox);
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
  Boolean fox_eat = FALSE;
  
  assert((!pos->oldest_fox && !pos->hungriest_fox && pos->best_rabbit) ||
    (pos->hungriest_fox && pos->oldest_fox && !pos->best_rabbit) ||
    (pos->oldest_fox && !pos->hungriest_fox && !pos->best_rabbit) ||
    (!pos->oldest_fox && !pos->hungriest_fox && !pos->best_rabbit));
  
  if(pos->best_rabbit) { // so apareceram coelhos
    pos->obj = (Object*)pos->best_rabbit;
    pos->best_rabbit = NULL;
  } else if(pos->hungriest_fox && pos->oldest_fox) { // so apareceram raposas
    pos->obj = (Object*)pos->hungriest_fox;
    if(pos->hungriest_fox != pos->oldest_fox)
      position_add_free(pos, (Object*)pos->oldest_fox);
      
    pos->oldest_fox = NULL;
    pos->hungriest_fox = NULL;
  } else if(pos->oldest_fox) { // apareceram coelhos e raposas!
    pos->oldest_fox->last_food = 0;
    fox_eat = TRUE;

    pos->obj = (Object*)pos->oldest_fox;
    pos->oldest_fox = NULL;
  } else {
    // nada
    pos->obj = NULL;
  }
  
  Object* obj = pos->obj;
  
  if(obj) {
    switch(obj->type) {
      case RABBIT:
      {
        Rabbit* rabbit = (Rabbit*)obj;
        
        if(rabbit->procreate) {
          rabbit->procreate = FALSE;
          rabbit->last_procreation = 0;
        } else
          rabbit->last_procreation++;
      }
      break;
      case FOX:
      {
        Fox* fox = (Fox*)obj;
        
        if(!fox_eat)
          fox->last_food++;
        
        if(fox->last_food >= map->ger_alim_raposas) {
          // morreu
          position_add_free(pos, (Object*)fox);
          pos->obj = NULL;
        } else {
          if(fox->procreate) {
            fox->procreate = FALSE;
            fox->last_procreation = 0;
          } else
            fox->last_procreation++;
        }
      }
      break;
    }
  }
  
  position_clean_free(pos, pos->obj);
}
