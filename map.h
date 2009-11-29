
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
  int* rows_ger_first_pass;
  int* rows_ger_second_pass;
  Boolean proceed_first_pass;
  Boolean proceed_second_pass;
  Boolean the_end;
  
  // main mutex and condition variable
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  
  // stats stuff
  pthread_mutex_t mtx_rabbit_deaths;
  pthread_mutex_t mtx_fox_deaths;
  pthread_mutex_t mtx_rabbit_reprod;
  pthread_mutex_t mtx_fox_reprod;
  int rabbit_reprod;
  int fox_reprod;
  int rabbit_deaths;
  int fox_deaths;

  Position *matrix;
} Map;

#define map_position_at(MAP, X, Y) ((MAP)->matrix + (X) * (MAP)->col + (Y))
#define map_position_at_coord(MAP, COORD) map_position_at(MAP, Coord_x(COORD), Coord_y(COORD))
#define map_stat_inc(MAP, STAT, VAL)         \
  {                                          \
    pthread_mutex_lock(&(MAP)->mtx_##STAT);   \
    (MAP)->STAT += VAL;                     \
    pthread_mutex_unlock(&(MAP)->mtx_##STAT); \
  }
#define map_rabbit_death_inc(MAP, VAL) map_stat_inc(MAP, rabbit_deaths, VAL)
#define map_rabbit_reprod_inc(MAP, VAL) map_stat_inc(MAP, rabbit_reprod, VAL)
#define map_fox_death_inc(MAP, VAL) map_stat_inc(MAP, fox_deaths, VAL)
#define map_fox_reprod_inc(MAP, VAL) map_stat_inc(MAP, fox_reprod, VAL)

Map*    map_read(FILE *fp);
void    map_print(Map* map);
void    map_free(Map* map);
Boolean map_inside(Map* map, int x, int y);
Boolean map_inside_coord(Map* map, Coord coord);
int     map_next_row(Map* map, int row);
void    map_statistics(Map* map);
void    map_output(Map* map, FILE* fp);

extern Map* map;

#endif
