
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
  
  int next_row_first_pass;
  int next_row_second_pass;
  int generation_first_pass;
  int generation_second_pass;
  int* rows_ger_first_pass;
  int* rows_ger_second_pass;
  
  Boolean proceed_first_pass;
  Boolean proceed_second_pass;
  Boolean the_end;
  Boolean more_first_pass;
  Boolean more_second_pass;
  
  // main mutex and condition variable
  pthread_mutex_t mutex;
  pthread_cond_t cond;

  Position *matrix;
} Map;

#define map_position_at(MAP, X, Y) ((MAP)->matrix + ((X) * (MAP)->col) + (Y))
#define map_position_at_coord(MAP, COORD) map_position_at(MAP, Coord_x(COORD), Coord_y(COORD))

Map*    map_read(FILE *fp);
void    map_print(Map* map, FILE* fp);
void    map_free(Map* map);
Boolean map_inside(Map* map, int x, int y);
Boolean map_inside_coord(Map* map, Coord coord);
int     map_next_row(Map* map, int row, int inc);
void    map_statistics(Map* map);
void    map_output(Map* map, FILE* fp);
Map*    map_generate(int rows, int cols, int ger_proc_coelhos, int ger_proc_raposas, int ger_alim_raposas, int n_ger, int density);
void    map_write(Map* map, FILE* fp);

#endif
