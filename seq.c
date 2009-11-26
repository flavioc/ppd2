
#include <stdlib.h>
#include <string.h>

#include "map.h"

static Boolean free_pos[4];
static Map* map = NULL;
static int ger;

#define CLEAR_FREE() memset(free_pos, TRUE, sizeof(free_pos))

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
    position_add_object(pos, pos->obj);
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
    
    position_add_object(pos, (Object*)object_new_rabbit(Coord_x(tmp_coord), Coord_y(tmp_coord)));
  }
  
  position_add_object(new_pos, (Object*)rabbit);
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
    
    if(tmp_pos->obj && tmp_pos->obj->type == RABBIT) {
      with_rabbits[i] = TRUE;
      ++rabbits;
    }
  }
  
  if(total_free == 0) {
    // não há casas livres, manter...
    position_add_object(pos, pos->obj);
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
    // vai procriar
    
    fox->last_procreation = 0;
    
    position_add_object(pos, (Object*)object_new_fox(Coord_x(tmp_coord), Coord_y(tmp_coord)));
  }
  
  position_add_object(other, (Object*)fox);
}

static void
resolve_conflict(Position* pos)
{
  ObjectList* list = pos->list_start;
  
  Rabbit* best_rabbit = NULL;
  Fox* oldest_fox = NULL;
  Fox* hungriest_fox = NULL;
  Fox* fox;
  Rabbit *rabbit;
  Object* obj;
  
  while(list) {
    obj = ObjectList_elem(list);
    
    switch(obj->type) {
      case FOX:
        fox = (Fox*)obj;
      
        if(!best_rabbit)
          if(!hungriest_fox || hungriest_fox->last_food > fox->last_food)
            hungriest_fox = fox;
          
        if(!oldest_fox || oldest_fox->last_procreation > fox->last_procreation)
          oldest_fox = fox;
          
      break;
      case RABBIT:
        if(!oldest_fox && !hungriest_fox) {
          rabbit = (Rabbit*)obj;
          
          if(!best_rabbit || rabbit->last_procreation > best_rabbit->last_procreation)
            best_rabbit = rabbit;
        }
      break;
    }
    
    list = ObjectList_next(list);
  }
  Object* dont_remove = NULL;
  
  if(!oldest_fox && best_rabbit) { // no foxes, only rabbits
    dont_remove = (Object*)best_rabbit;
  } else if(hungriest_fox && !best_rabbit) { // only foxes
    dont_remove = (Object*)hungriest_fox;
  } else if(oldest_fox && best_rabbit) { // foxes and rabbits
    dont_remove = (Object*)oldest_fox;
    oldest_fox->last_food = 0;
  }
  
  // delete other objects
  list = pos->list_start;
  
  while(list) {
    obj = ObjectList_elem(list);
    
    if(obj != dont_remove)
      free(obj);
    
    list = ObjectList_next(list);
  }
  
  if(dont_remove)
    pos->obj = dont_remove;
}

static void
simulate_gen()
{
  int i, j;
  
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Position *pos = Position_at(map, i, j);
      
      if(pos->obj) {
        switch(pos->obj->type) {
          case RABBIT:
            simulate_rabbit(pos); break;
          case FOX:
            simulate_fox(pos); break;
        }
        
        pos->obj = NULL;
      }
    }
  
  // resolver conflitos
  for(i = 0; i < map->lin; ++i) {
    for(j = 0; j < map->col; ++j) {
      Position* pos = Position_at(map, i, j);
      
      if(position_has_list(pos)) {
        resolve_conflict(pos);
      }
    }  
  }
  
  // libertar listas
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j)
      position_free_list(Position_at(map, i, j));
}

static void
simulate()
{
  for(ger = 0; ger < map->n_ger; ++ger) {
    simulate_gen();
    printf("Generation %d:\n", ger + 1);
    map_print(map);
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
