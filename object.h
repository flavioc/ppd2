#ifndef OBJECT_H
#define OBJECT_H

#include "utils.h"

typedef enum {
  RABBIT,
  FOX
} ObjectType;

typedef struct {
  ObjectType type;
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

Rabbit*     object_new_rabbit();
Fox*        object_new_fox();

#endif