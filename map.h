
#ifndef MAP_H
#define MAP_H

#include <stdio.h>

#include "utils.h"

typedef enum {
  RABBIT,
  FOX
} ObjectType;

typedef struct {
  ObjectType type;
  Coord coord;
} Object;

typedef struct {
  Object comm;
  int last_procreation;
} Rabbit;

typedef struct {
  Object comm;
  int last_procreation;
  int last_food;
} Fox;

#define Object_type(OBJ) (OBJ)->comm.type
#define Object_coord(OBJ) (OBJ)->comm.coord
#define Object_x(OBJ) Coord_x(Object_coord(OBJ))
#define Object_y(OBJ) Coord_y(Object_coord(OBJ))

typedef struct {
  Boolean is_rock;
  Object* obj;
  Object* new;
} Position;

typedef struct {
  int ger_proc_coelhos;
  int ger_proc_raposas;
  int ger_alim_raposas;
  int n_ger;
  int lin;
  int col;
  
  int rabbit_reprod;
  int rabbit_collision;

  Position *matrix;
} Map;

Rabbit* map_new_rabbit(int x, int y);
Fox*    map_new_fox(int x, int y);
Map*    map_read(FILE *fp);
void    map_print(Map* map);
void    map_free(Map* map);
void    map_change(Map* map);
Boolean map_inside(Map* map, Coord coord);
void    map_statistics(Map* map);

#define Position_at(MAP, X, Y) ((MAP)->matrix + (X) * (MAP)->col + (Y))
#define Position_at_coord(MAP, COORD) Position_at(MAP, Coord_x(COORD), Coord_y(COORD))

#endif
