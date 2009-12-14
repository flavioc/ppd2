
#include <stdlib.h>

#include "object.h"

Rabbit*
object_new_rabbit(void)
{
  Rabbit* ret = (Rabbit*)malloc(sizeof(Rabbit));

  Object_type(ret) = RABBIT;
  ret->last_procreation = 0;
  ret->procreate = FALSE;

  return ret;
}

Fox*
object_new_fox(void)
{
  Fox* ret = (Fox*)malloc(sizeof(Fox));

  Object_type(ret) = FOX;

  ret->last_procreation = 0;
  ret->last_food = 0;
  ret->procreate = FALSE;

  return ret;
}