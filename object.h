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

typedef struct lista_objects {
  Object* obj;
  struct lista_objects* next;
} ObjectList;

#define Object_type(OBJ) ((OBJ)->comm.type)
#define Object_coord(OBJ) ((OBJ)->comm.coord)

Rabbit*     object_new_rabbit(void);
Fox*        object_new_fox(void);
ObjectList* object_list_new(Object* obj, ObjectList* next);

#endif