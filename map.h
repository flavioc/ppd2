
#ifndef MAP_H
#define MAP_H

typedef enum {
  ROCK,
  RABBIT,
  FOX
} ObjectType;

typedef struct {
  ObjectType type;
  unsigned int x;
  unsigned int y;
} Object;

typedef struct {
  Object comm;
} Rabbit;

typedef struct {
  Object comm;
} Fox;

typedef struct {
  Object comm;
} Rock;

#define Object_type(OBJ) (OBJ)->comm.type
#define Object_x(OBJ) (OBJ)->comm.x
#define Object_y(OBJ) (OBJ)->comm.y

typedef struct {
  unsigned int ger_proc_coelhos;
  unsigned int ger_proc_raposas;
  unsigned int ger_alim_raposas;
  unsigned int n_ger;
  unsigned int lin;
  unsigned int col;

  Object **matrix;
} Map;

Rabbit* map_new_rabbit();
Fox*    map_new_fox();
Rock*   map_new_rock();

#endif
