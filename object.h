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

typedef struct object_list {
  Object* elem;
  struct object_list* next;
} ObjectList;

#define ObjectList_next(LIST) (LIST)->next
#define ObjectList_elem(LIST) (LIST)->elem

Rabbit*     object_new_rabbit(int x, int y);
Fox*        object_new_fox(int x, int y);
ObjectList* object_add_list(ObjectList* start, Object* obj);
ObjectList* object_new_list(Object* obj);

#endif