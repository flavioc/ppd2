
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "map.h"
#include "thread.h"

static Map* map = NULL;

static void
simulate_gen(ThreadData* data, int ger)
{
  int i, j;
  Coord coord = {0, 0};
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Coord_x(coord) = i;
      Coord_y(coord) = j;
      printf("1st: %d %d %d\n", ger, i, j);
      Position* pos = map_position_at(map, i, j);
      thread_simulate_position(data, map, pos, coord, ger);
    }
  
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Position* pos = map_position_at(map, i, j);
      printf("2st: %d %d %d\n", ger, i, j);
      thread_resolve_conflict(pos);
    }
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
main(void)
{
  map = map_read(stdin);
  
  if(!map) {
    fprintf(stderr, "Could not read map file\n");
    return EXIT_FAILURE;
  }
  
  map_print(map);
  
  simulate();
  
  map_print(map);
  //map_statistics(map);
  map_output(map, stdout);
  map_free(map);
  
  return EXIT_SUCCESS;
}
