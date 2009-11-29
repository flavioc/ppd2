
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "map.h"
#include "thread.h"

static int ger;
static ThreadData data;

static void
simulate_rabbit(Coord coord, Position* pos)
{
  Rabbit* rabbit = (Rabbit*)pos->obj;
  int i, total_free = ADJACENT;
  Coord tmp_coord;
  Position* tmp_pos;

  for(i = 0; i < ADJACENT; ++i) {
    tmp_coord = coord_at_direction(coord, i);
    
    if(!map_inside_coord(map, tmp_coord)) {
      data.free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    // verificar se existe uma raposa ou rocha
    tmp_pos = data.positions[i] = map_position_at_coord(map, tmp_coord);
    
    if(tmp_pos->is_rock || tmp_pos->obj)
    {
      data.free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    data.free_pos[i] = TRUE;
  }
  
  rabbit->last_procreation++;
  
  if(total_free == 0) {
    // não há casas livres, manter...
    position_move_rabbit(pos, rabbit);
    return;
  }
  
  // seleccionar casa a mover
  int index = (ger + Coord_x(coord) + Coord_y(coord)) % total_free;
  int real_index;
  
  for(i = 0; i < ADJACENT; ++i)
    if(data.free_pos[i])
      if(!index--) {
        real_index = i;
        break;
      }
  
  if(rabbit->last_procreation >= map->ger_proc_coelhos) {
    // vai reproduzir-se...
    
    rabbit->last_procreation = 0;
    map_rabbit_reprod_inc(map, 1);
    
    position_move_rabbit(pos, object_new_rabbit());
  }
  
  position_move_rabbit(data.positions[real_index], rabbit);
}

static void
simulate_fox(Coord coord, Position* pos)
{
  Fox* fox = (Fox*)pos->obj;
  
  fox->last_food++;
  
  if(fox->last_food >= map->ger_alim_raposas) {
    // morreu
    free(fox);
    map_fox_death_inc(map, 1);
    return;
  }
  
  fox->last_procreation++;
  
  int i, total_free = ADJACENT, rabbits = 0;
  Coord tmp_coord;
  Position* tmp_pos;
  
  for(i = 0; i < ADJACENT; ++i) {
    tmp_coord = coord_at_direction(coord, i);
    
    if(!map_inside_coord(map, tmp_coord)) {
      data.free_pos[i] = FALSE;
      --total_free;
      data.with_rabbits[i] = FALSE;
      continue;
    }
    
    tmp_pos = data.positions[i] = map_position_at_coord(map, tmp_coord);
    
    if(tmp_pos->is_rock ||
      (tmp_pos->obj && tmp_pos->obj->type == FOX))
    {
      data.free_pos[i] = FALSE;
      --total_free;
      data.with_rabbits[i] = FALSE;
      continue;
    }
    
    if(tmp_pos->obj && tmp_pos->obj->type == RABBIT) {
      data.with_rabbits[i] = TRUE;
      // no need to set free_pos here...
      ++rabbits;
      continue;
    }
    
    data.free_pos[i] = TRUE;
    data.with_rabbits[i] = FALSE;
  }
  
  if(total_free == 0) {
    // não há casas livres, manter...
    position_move_fox(pos, fox);
    return;
  }
 
  int real_index, index = ger + Coord_x(coord) + Coord_y(coord);
  Boolean *bool_array;
 
  if(rabbits > 0) {
    index %= rabbits;
    bool_array = data.with_rabbits;
  } else {
    bool_array = data.free_pos;
    index %= total_free;
  }
  
  for(i = 0; i < ADJACENT; ++i)
    if(bool_array[i])
      if(!index--) {
        real_index = i;
        break;
      }
  
  if(fox->last_procreation >= map->ger_proc_raposas) {
    // vai procriar
    fox->last_procreation = 0;
    map_fox_reprod_inc(map, 1);
    position_move_fox(pos, object_new_fox());
  }
  
  position_move_fox(data.positions[real_index], fox);
}

static void
resolve_conflict(Position* pos)
{
  if(pos->best_rabbit) { // so apareceram coelhos
    pos->obj = (Object*)pos->best_rabbit;
    pos->best_rabbit = NULL;
  } else if(pos->hungriest_fox && pos->oldest_fox) { // so apareceram raposas
    pos->obj = (Object*)pos->oldest_fox;
    
    if(pos->hungriest_fox != pos->oldest_fox)
      position_add_free(pos, (Object*)pos->hungriest_fox);
    
    map_fox_death_inc(map, position_clean_free(pos, pos->obj));
    
    pos->oldest_fox = NULL;
    pos->hungriest_fox = NULL;
  } else if(pos->oldest_fox) { // apareceram coelhos e raposas!
    pos->obj = (Object*)pos->oldest_fox;
    map_fox_death_inc(map, position_clean_free(pos, pos->obj));
    pos->oldest_fox = NULL;
  }
}

static void
simulate_gen()
{
  int i, j;
  Coord coord;
  
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Position *pos = map_position_at(map, i, j);
      
      Coord_x(coord) = i;
      Coord_y(coord) = j;
      
      if(pos->obj) {
        switch(pos->obj->type) {
          case RABBIT:
            simulate_rabbit(coord, pos); break;
          case FOX:
            simulate_fox(coord, pos); break;
        }
      }
    }
  
  // resolver conflitos
  for(i = 0; i < map->lin; ++i) {
    for(j = 0; j < map->col; ++j) {
      Position* pos = map_position_at(map, i, j);
  
      pos->obj = NULL;
      resolve_conflict(pos);
      assert(pos->best_rabbit == NULL);
      assert(pos->oldest_fox == NULL);
      assert(pos->hungriest_fox == NULL);
    } 
  }
}

static void
simulate()
{
  for(ger = 0; ger < map->n_ger; ++ger) {
    simulate_gen();
    //printf("Generation %d:\n", ger + 1);
    //map_print(map);
  }
}

int
main()
{
  map = map_read(stdin);
  
  map_print(map);
  
  simulate();
  
  printf("Map end:\n");
  map_print(map);
  map_statistics(map);
  map_output(map, stdout);
  map_free(map);
  
  return EXIT_SUCCESS;
}
