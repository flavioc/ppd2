
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "map.h"
#include "thread.h"

static ThreadData data;

static void
simulate_gen(int ger)
{
  int i, j;
  Coord coord;
  Position* pos;
  
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Coord_x(coord) = i;
      Coord_y(coord) = j;
      
      thread_simulate_position(&data, map_position_at(map, i, j), coord, ger);
    }
  
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      pos = map_position_at(map, i, j);
      thread_resolve_conflict(pos);
    } 
}

static void
simulate()
{
  int ger;
  
  for(ger = 0; ger < map->n_ger; ++ger) {
    simulate_gen(ger);
  }
}

int
main()
{
  map = map_read(stdin);
  
  map_print(map);
  
  simulate();
  
  map_print(map);
  //map_statistics(map);
  //map_output(map, stdout);
  map_free(map);
  
  return EXIT_SUCCESS;
}
