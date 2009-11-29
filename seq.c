
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "map.h"

static Boolean free_pos[4];
static int ger;

#define CLEAR_FREE() memset(free_pos, TRUE, sizeof(free_pos))

static void
simulate_rabbit(Coord coord, Position* pos)
{
  Rabbit* rabbit = (Rabbit*)pos->obj;
  Position* tmp_pos;
  int total_free = 4, i;
  Coord tmp_coord;
  
  CLEAR_FREE();
  
  for(i = 0; i < 4; ++i) {
    tmp_coord = coord_at_direction(coord, i);
    
    if(!map_inside_coord(map, tmp_coord)) {
      free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    // verificar se existe uma raposa ou rocha
    tmp_pos = map_position_at_coord(map, tmp_coord);
    
    if(tmp_pos->is_rock ||
      (tmp_pos->obj && tmp_pos->obj->type == FOX))
    {
      free_pos[i] = FALSE;
      --total_free;
    }
  }
  
  rabbit->last_procreation++;
  
  if(total_free == 0) {
    // não há casas livres, manter...
    position_move_rabbit(pos, rabbit);
    return;
  }
  
  // seleccionar casa a mover
  int index = (ger + Coord_x(coord) + Coord_y(coord)) % total_free;
  
  for(i = 0; i < 4; ++i) {
    if(free_pos[i]) {
      if(!index) {
        tmp_coord = coord_at_direction(coord, i);
        break;
      }
      --index;
    }
  }
  
  Position* new_pos = map_position_at_coord(map, tmp_coord);
  
  if(rabbit->last_procreation >= map->ger_proc_coelhos) {
    // vai reproduzir-se...
    
    rabbit->last_procreation = 0;
    map_rabbit_reprod_inc(map, 1);
    
    position_move_rabbit(pos, object_new_rabbit(Coord_x(tmp_coord), Coord_y(tmp_coord)));
  }
  
  position_move_rabbit(new_pos, rabbit);
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
  
  Coord tmp_coord;
  int i, total_free = 4, rabbits = 0;
  Boolean with_rabbits[4];
  Position* tmp_pos;
  
  memset(with_rabbits, FALSE, sizeof(with_rabbits));
  
  CLEAR_FREE();
  
  for(i = 0; i < 4; ++i) {
    tmp_coord = coord_at_direction(coord, i);
    
    if(!map_inside_coord(map, tmp_coord)) {
      free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    tmp_pos = map_position_at_coord(map, tmp_coord);
    
    if(tmp_pos->is_rock ||
      (tmp_pos->obj && tmp_pos->obj->type == FOX))
    {
      free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    if(tmp_pos->obj && tmp_pos->obj->type == RABBIT) {
      with_rabbits[i] = TRUE;
      ++rabbits;
    }
  }
  
  if(total_free == 0) {
    // não há casas livres, manter...
    position_move_fox(pos, fox);
    return;
  }
 
  Position* other;
  int index;
 
  if(rabbits > 0) {
    // escolher uma casa com coelho
    index = (ger + Coord_x(coord) + Coord_y(coord)) % rabbits;
   
    for(i = 0; i <= 4; ++i) {
      if(with_rabbits[i]) {
        if(!index) {
          tmp_coord = coord_at_direction(coord, i);
          break;
        }
       
        --index;
      }
    }
  } else {
    index = (ger + Coord_x(coord) + Coord_y(coord)) % total_free;
    
    for(i = 0; i <= 4; ++i) {
      if(free_pos[i]) {
        if(!index) {
          tmp_coord = coord_at_direction(coord, i);
          break;
        }
        
        --index;
      }
    }
  }
  
  other = map_position_at_coord(map, tmp_coord);
  
  if(fox->last_procreation >= map->ger_proc_raposas) {
    // vai procriar
    fox->last_procreation = 0;
    map_fox_reprod_inc(map, 1);
    position_move_fox(pos, object_new_fox(Coord_x(tmp_coord), Coord_y(tmp_coord)));
  }
  
  position_move_fox(other, fox);
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
      
      //printf("Current free: %d %d %d\n", i, j, pos->current_free);
      //assert(pos->current_free == 0);
      
      if(pos->obj) {
        switch(pos->obj->type) {
          case RABBIT:
            simulate_rabbit(coord, pos); break;
          case FOX:
            simulate_fox(coord, pos); break;
        }
        
        pos->obj = NULL;
      }
    }
  
  // resolver conflitos
  for(i = 0; i < map->lin; ++i) {
    for(j = 0; j < map->col; ++j) {
      Position* pos = map_position_at(map, i, j);
      
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
