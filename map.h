
#ifndef MAP_H
#define MAP_H

#include <stdio.h>

#include "config.h"
#include "utils.h"
#include "position.h"

typedef struct {
  int ger_proc_coelhos;
  int ger_proc_raposas;
  int ger_alim_raposas;
  int n_ger;
  int lin;
  int col;
  
  int rabbit_reprod;
  int fox_reprod;
  int rabbit_deaths;
  int fox_deaths;

  Position *matrix;
} Map;

#define Position_at(MAP, X, Y) ((MAP)->matrix + (X) * (MAP)->col + (Y))
#define Position_at_coord(MAP, COORD) Position_at(MAP, Coord_x(COORD), Coord_y(COORD))

Map*    map_read(FILE *fp);
void    map_print(Map* map);
void    map_free(Map* map);
Boolean map_inside(Map* map, int x, int y);
Boolean map_inside_coord(Map* map, Coord coord);
void    map_statistics(Map* map);
void    map_output(Map* map, FILE* fp);

#endif
