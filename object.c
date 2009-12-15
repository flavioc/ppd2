
#include <stdlib.h>

#include "object.h"

Rabbit*
object_new_rabbit(void)
{
  Rabbit* ret = (Rabbit*)malloc(sizeof(Rabbit));

  Object_type(ret) = RABBIT;
  ret->last_procreation = 0;

  return ret;
}

Fox*
object_new_fox(void)
{
  Fox* ret = (Fox*)malloc(sizeof(Fox));

  Object_type(ret) = FOX;

  ret->last_procreation = 0;
  ret->last_food = 0;

  return ret;
}

ObjectList*
object_list_new(Object* obj, ObjectList* next)
{
  ObjectList *new = (ObjectList*)malloc(sizeof(struct lista_objects));
  
  new->obj = obj;
  new->next = next;
  
  return new;
}