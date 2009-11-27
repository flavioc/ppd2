#ifndef OBJECT_H
#define OBJECT_H

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

Rabbit*     object_new_rabbit(int x, int y);
Fox*        object_new_fox(int x, int y);

#endif