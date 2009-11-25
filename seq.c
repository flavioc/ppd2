
#include <stdlib.h>
#include <string.h>

#include "map.h"

static Boolean free_pos[4];
static Map* map = NULL;
static int ger;

#define CLEAR_FREE() memset(free_pos, TRUE, sizeof(free_pos))

static void
move_rabbit(Position* new_pos, Rabbit* rabbit)
{
  Object* current = new_pos->new;
  Boolean change = TRUE;
  
  if(current && current->type == RABBIT) {
    // ja contem um coelho!
    // fica o que tiver mais idade
    Rabbit* cur_rabbit = (Rabbit*)current;
    
    map->rabbit_collision++;
    
    if(rabbit->last_procreation < cur_rabbit->last_procreation)
      change = FALSE;
  }
  
  if(change)
    new_pos->new = (Object*)rabbit;
}

static void
simulate_rabbit(Position* pos)
{
  Rabbit* rabbit = (Rabbit*)pos->obj;
  Coord coord = Object_coord(rabbit);
  Position* tmp_pos;
  int total_free = 4, i;
  Coord tmp_coord;
  
  CLEAR_FREE();
  
  for(i = 0; i < 4; ++i) {
    tmp_coord = coord_at_direction(coord, i);
    
    if(!map_inside(map, tmp_coord)) {
      free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    // verificar se existe uma raposa ou rocha
    tmp_pos = Position_at_coord(map, tmp_coord);
    
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
    pos->new = pos->obj;
    
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
  
  Position* new_pos = Position_at_coord(map, tmp_coord);
  
  if(rabbit->last_procreation >= map->ger_proc_coelhos) {
    // vai reproduzir-se...
    
    rabbit->last_procreation = 0;
    map->rabbit_reprod++;
    
    move_rabbit(pos, map_new_rabbit(Coord_x(tmp_coord), Coord_y(tmp_coord)));
  }
  
  move_rabbit(new_pos, rabbit);
}

static void
move_fox(Position* pos, Fox* fox)
{
  Object* current = pos->new;
  
  if(!current) {
    pos->new = (Object*)fox;
    return;
  }
  
  if(current->type == FOX) {
    // fica
  }
}

static void
simulate_fox(Position* pos)
{
  Fox* fox = (Fox*)pos->obj;
  
  fox->last_food++;
  
  if(fox->last_food >= map->ger_alim_raposas) {
    // morreu
    return;
  }
  
  fox->last_procreation++;
  
  Coord coord = Object_coord(fox);
  Coord tmp_coord;
  int i, total_free = 4, rabbits = 0;
  Boolean with_rabbits[4];
  Position* tmp_pos;
  
  memset(with_rabbits, FALSE, sizeof(with_rabbits));
  
  CLEAR_FREE();
  
  for(i = 0; i < 4; ++i) {
    tmp_coord = coord_at_direction(coord, i);
    
    if(!map_inside(map, tmp_coord)) {
      free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    tmp_pos = Position_at_coord(map, tmp_coord);
    
    if(tmp_pos->is_rock ||
      (tmp_pos->obj && tmp_pos->obj->type == FOX))
    {
      free_pos[i] = FALSE;
      --total_free;
      continue;
    }
    
    if(tmp_pos->new && tmp_pos->new->type == RABBIT) {
      with_rabbits[i] = TRUE;
      ++rabbits;
    }
  }
  
  if(total_free == 0) {
    // não há casas livres, manter...
    pos->new = pos->obj;
   
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
  
  other = Position_at_coord(map, tmp_coord);
  
  if(fox->last_procreation >= map->ger_proc_raposas) {
    // vai procrear
    
    fox->last_procreation = 0;
    
    Fox* child = map_new_fox(Coord_x(tmp_coord), Coord_y(tmp_coord));
    move_fox(pos, child);
  }
  
  move_fox(other, fox);
}

static void
simulate_gen()
{
  int i, j;
  
  // simulate rabbits
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Position *pos = Position_at(map, i, j);
      
      if(pos->obj && pos->obj->type == RABBIT) {
        simulate_rabbit(pos);
      }
    }
  
  // simulate foxes
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Position *pos = Position_at(map, i, j);
      
      if(pos->obj && pos->obj->type == FOX)  {
        simulate_fox(pos);
      }
    }
  
  map_change(map);
}

static void
simulate()
{
  for(ger = 0; ger < map->n_ger; ++ger) {
    simulate_gen();
    printf("Generation %d:\n", ger + 1);
    map_print(map);
    printf("========================\n");
  }
}

int
main()
{
  map = map_read(stdin);
  
  printf("Map start:\n");
  map_print(map);
  
  simulate();
  
  printf("Map end:\n");
  map_print(map);
  map_statistics(map);
  map_free(map);
  
  return EXIT_SUCCESS;
}
