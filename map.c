
#include <stdlib.h>

#include "map.h"

Rabbit*
map_new_rabbit()
{
  Rabbit* ret = (Rabbit*)malloc(sizeof(Rabbit));

  Object_type(ret) = RABBIT;

  return ret;
}

Fox*
map_new_fox()
{
  Fox* ret = (Fox*)malloc(sizeof(Fox));

  Object_type(ret) = FOX;

  return ret;
}

Rock*
map_new_rock()
{
  Rock *ret = (Rock*)malloc(sizeof(Rock));

  Object_type(ret) = ROCK;

  return ret;
}
