
#include <stdlib.h>
#include <string.h>

#include "map.h"

Rabbit*
map_new_rabbit(int x, int y)
{
  Rabbit* ret = (Rabbit*)malloc(sizeof(Rabbit));

  Object_type(ret) = RABBIT;
  Object_x(ret) = x;
  Object_y(ret) = y;
  ret->last_procreation = 0;

  return ret;
}

Fox*
map_new_fox(int x, int y)
{
  Fox* ret = (Fox*)malloc(sizeof(Fox));

  Object_type(ret) = FOX;
  Object_x(ret) = x;
  Object_y(ret) = y;
  ret->last_procreation = 0;
  ret->last_food = 0;

  return ret;
}

void
map_free(Map* map)
{
  if(!map)
    return;
    
  int i, j;
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Position *pos = Position_at(map, i, j);
      
      if(pos && pos->obj)
        free(pos->obj);
    }
  
  if(map->matrix)
    free(map->matrix);
    
  free(map);
}

Map*
map_read(FILE* fp)
{
  int ger_proc_coelhos, ger_proc_raposas, ger_alim_raposas;
  int n_ger, lin, col, n_objs;
  
  int ret = fscanf(fp, "%d\n%d\n%d\n%d\n%d %d\n%d\n",
    &ger_proc_coelhos, &ger_proc_raposas,
    &ger_alim_raposas, &n_ger, &lin, &col,
    &n_objs);
  
  if(ret != 7)
    return NULL;
  
  int i, j;
  Map* map = (Map*)malloc(sizeof(Map));
  
  map->ger_proc_coelhos = ger_proc_coelhos;
  map->ger_proc_raposas = ger_proc_raposas;
  map->ger_alim_raposas = ger_alim_raposas;
  map->n_ger = n_ger;
  map->lin = lin;
  map->col = col;
  map->rabbit_reprod = 0;
  map->rabbit_collision = 0;
  map->matrix = (Position*)malloc(sizeof(Position) * lin * col);
  
  for(i = 0; i < lin; ++i)
    for(j = 0; j < col; ++j) {
      Position *pos = Position_at(map, i, j);
      
      pos->obj = pos->new = NULL;
      pos->is_rock = FALSE;
    }
  
  char type[7];
  int x, y;
  Object* obj;
  
  for(i = 0; i < n_objs; ++i) {
    if(fscanf(fp, "%6s %d %d", type, &x, &y) != 3) {
      map_free(map);
      return NULL;
    }
    printf("Read %s %d %d\n", type, x, y);
    
    if(strcmp(type, "ROCHA") == 0) {
      Position_at(map, x, y)->is_rock = TRUE;
      continue;
    }
    
    if(strcmp(type, "RAPOSA") == 0)
      obj = (Object*)map_new_fox(x, y);
    else if(strcmp(type, "COELHO") == 0)
      obj = (Object*)map_new_rabbit(x, y);
    else
      obj = NULL;
      
    if(!obj) {
      map_free(map);
      return NULL;
    }
    
    Position_at(map, x, y)->obj = obj;
  }
  
  return map;
}

static void
print_line(int size)
{
  int i;
  
  for(i = 0; i < size; ++i)
    printf("=");
    
  printf("\n");
}

void
map_print(Map* map)
{
  int i, j;
  
  print_line(map->col + 2);
  
  for(i = 0; i < map->lin; ++i) {
    printf("|");
    for(j = 0; j < map->col; ++j) {
      Position *pos = Position_at(map, i, j);
      
      if(pos->is_rock) {
        printf("-");
        continue;
      }
      
      if(pos->obj)
        switch(pos->obj->type) {
          case RABBIT:
            printf("C"); break;
          case FOX:
            printf("F"); break;
        }
      else
        printf(" ");
    }
    printf("|\n");
  }
  
  print_line(map->col + 2);
}

void
map_change(Map* map)
{
  int i, j;
  
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Position_at(map, i, j)->obj = Position_at(map, i, j)->new;
      Position_at(map, i, j)->new = NULL;
    }
}

void
map_statistics(Map* map)
{
  int total_rabbits, total_foxes;
  int i, j;
  
  total_rabbits = total_foxes = 0;
  
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Position* pos = Position_at(map, i, j);
      
      if(pos->obj) {
        switch(pos->obj->type) {
          case RABBIT: ++total_rabbits; break;
          case FOX: ++total_foxes; break;
        }
      }
    }
    
  printf("===================================\n");
  printf("  || FOXES: %d\tRABBITS: %d\n", total_foxes, total_rabbits);
  printf("  || RABBIT: +%d  -%d\n", map->rabbit_reprod, map->rabbit_collision);
  printf("===================================\n");
}

Boolean
map_inside(Map* map, Coord coord)
{
  if(Coord_x(coord) < 0)
    return FALSE;
    
  if(Coord_y(coord) < 0)
    return FALSE;
  
  if(Coord_x(coord) >= map->lin)
    return FALSE;
    
  if(Coord_y(coord) >= map->col)
    return FALSE;
    
  return TRUE;
}