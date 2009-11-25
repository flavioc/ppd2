
#include "utils.h"

Coord
coord_at_direction(Coord coord, Direction dir)
{
  Coord ret;
  int x = Coord_x(coord);
  int y = Coord_y(coord);
  
  switch(dir) {
    case DIR_UP:
      Coord_x(ret) = x - 1;
      Coord_y(ret) = y;
      break;
    case DIR_RIGHT:
      Coord_x(ret) = x;
      Coord_y(ret) = y + 1;
      break;
    case DIR_DOWN:
      Coord_x(ret) = x + 1;
      Coord_y(ret) = y;
      break;
    case DIR_LEFT:
      Coord_x(ret) = x;
      Coord_y(ret) = y - 1;
      break;
  }
  
  return ret;
}