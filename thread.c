
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
      
  assert(real_index != -1);
  assert(real_index >= 0 && real_index < 4);
  
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
  
  fox->last_food++;
  
  if(fox->last_food >= map->ger_alim_raposas) {
    pthread_mutex_lock(&pos->mutex);
    // morreu
    position_add_free(pos, (Object*)fox);
    pthread_mutex_unlock(&pos->mutex);
    return;
  }
  
  fox->last_procreation++;
  
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
    
    if(
    tmp_pos->is_rock ||
      (tmp_pos->obj && tmp_pos->obj->type == FOX))
    {
      data->free_pos[i] = FALSE;
      --total_free;
      data->with_rabbits[i] = FALSE;
      continue;
    }
    
    if(tmp_pos->obj && tmp_pos->obj->type == RABBIT) {
      data->with_rabbits[i] = TRUE;
      // no need to set free_pos here...
      ++rabbits;
      continue;
    }
    
    data->free_pos[i] = TRUE;
    data->with_rabbits[i] = FALSE;
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
    bool_array = data->with_rabbits;
  } else {
    bool_array = data->free_pos;
    index %= total_free;
  }
  
  assert(bool_array != NULL);
  
  for(i = 0; i < ADJACENT; ++i) {
    if(bool_array[i]) {
      if(!index--) {
        real_index = i;
        break;
      }
    }
  }
      
  assert(real_index >= 0 && real_index < ADJACENT);
  
  if(fox->last_procreation >= map->ger_proc_raposas) {
    // vai procriar
    fox->last_procreation = 0;
    position_move_fox(pos, object_new_fox());
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
  
  //printf("A simular %d %d na geração %d\n", Coord_x(coord), Coord_y(coord), ger);
}

void
thread_resolve_conflict(Position* pos)
{
  assert(pos != NULL);
  
  pos->obj = NULL;
  
  if(pos->best_rabbit) { // so apareceram coelhos
    pos->obj = (Object*)pos->best_rabbit;
    pos->best_rabbit = NULL;
  } else if(pos->hungriest_fox && pos->oldest_fox) { // so apareceram raposas
    pos->obj = (Object*)pos->oldest_fox;
    
    if(pos->hungriest_fox != pos->oldest_fox)
      position_add_free(pos, (Object*)pos->hungriest_fox);
    
    pos->oldest_fox = NULL;
    pos->hungriest_fox = NULL;
  } else if(pos->oldest_fox) { // apareceram coelhos e raposas!
    pos->obj = (Object*)pos->oldest_fox;
    pos->oldest_fox = NULL;
  }
  
  position_clean_free(pos, pos->obj);
  
  assert(pos->best_rabbit == NULL);
  assert(pos->hungriest_fox == NULL);
  assert(pos->oldest_fox == NULL);
}