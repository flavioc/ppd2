
#include <stdlib.h>

#include "object.h"

Rabbit*
object_new_rabbit(int x, int y)
{
  Rabbit* ret = (Rabbit*)malloc(sizeof(Rabbit));

  Object_type(ret) = RABBIT;
  Object_x(ret) = x;
  Object_y(ret) = y;
  ret->last_procreation = 0;

  return ret;
}

Fox*
object_new_fox(int x, int y)
{
  Fox* ret = (Fox*)malloc(sizeof(Fox));

  Object_type(ret) = FOX;
  Object_x(ret) = x;
  Object_y(ret) = y;

  ret->last_procreation = 0;
  ret->last_food = 0;

  return ret;
}

ObjectList*
object_new_list(Object* obj)
{
  ObjectList* new = (ObjectList*)malloc(sizeof(ObjectList));
  
  ObjectList_elem(new) = obj;
  ObjectList_next(new) = NULL;
  
  return new;
}

ObjectList*
object_add_list(ObjectList* start, Object* obj)
{
  ObjectList* new = object_new_list(obj);
  ObjectList_next(start) = new;
  
  return new;
}