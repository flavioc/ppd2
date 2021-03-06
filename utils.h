#ifndef UTILS_H
#define UTILS_H

typedef int Boolean;

#define FALSE 0
#define TRUE !FALSE

#define ADJACENT 4

typedef struct {
  int x;
  int y;
} Coord;

#define Coord_x(COORD) ((COORD).x)
#define Coord_y(COORD) ((COORD).y)

typedef enum {
  DIR_UP = 0,
  DIR_RIGHT,
  DIR_DOWN,
  DIR_LEFT
} Direction;
  
Coord coord_at_direction(Coord coord, Direction dir);  

#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))

int generate_random(int max);
long get_miliseconds(void);
   
#endif