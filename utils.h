#ifndef UTILS_H
#define UTILS_H

typedef int Boolean;

#define FALSE 0
#define TRUE !FALSE

typedef struct {
  int x;
  int y;
} Coord;

#define Coord_x(COORD) (COORD).x
#define Coord_y(COORD) (COORD).y

typedef enum {
  DIR_UP,
  DIR_RIGHT,
  DIR_DOWN,
  DIR_LEFT
} Direction;
  
Coord coord_at_direction(Coord coord, Direction dir);  
  
#endif