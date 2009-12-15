
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "map.h"
#include "thread.h"

static Map* map = NULL;

static inline void
simulate_gen(ThreadData* data, int ger)
{
  int i, j;
  Coord coord;
  Position* pos;
  
  for(i = 0; i < map->lin; ++i) {
    for(j = 0; j < map->col; ++j) {
      Coord_x(coord) = i;
      Coord_y(coord) = j;
      
      pos = map_position_at(map, i, j);
      thread_simulate_position(data, map, pos, coord, ger);
    }
  }
  
  for(i = 0; i < map->lin; ++i) {
    for(j = 0; j < map->col; ++j) {
      pos = map_position_at(map, i, j);
      thread_resolve_conflict(map, pos);
    }
  }
  
  //map_print(map, stdout);
}

static void
simulate(void)
{
  int ger;
  ThreadData data;
  
  for(ger = 0; ger < map->n_ger; ++ger) {
    simulate_gen(&data, ger);
  }
}

int
main(int argc, char** argv)
{
  (void)argc;
  (void)argv;
  
  map = map_read(stdin);
   
  if(!map) {
    fprintf(stderr, "Could not read map file\n");
    return EXIT_FAILURE;
  }
 
  //map_print(map);
  
  simulate();
  
  //map_print(map, stdout);
  //map_statistics(map);
  //map_write(map, stderr);
  map_output(map, stdout);
  map_free(map);
  
  return EXIT_SUCCESS;
}