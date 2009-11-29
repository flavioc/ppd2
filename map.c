
#include <stdlib.h>
#include <string.h>

#include "map.h"

Map* map = NULL;

void
map_free(Map* map)
{
  if(!map)
    return;
  
  if(map->matrix) {  
    int i, j;
    for(i = 0; i < map->lin; ++i)
      for(j = 0; j < map->col; ++j) {
        Position *pos = map_position_at(map, i, j);
      
        if(pos)
          position_free(pos);
      }
  
    free(map->matrix);
  }
  
  if(map->rows_ger_first_pass)
    free(map->rows_ger_first_pass);
    
  if(map->rows_ger_second_pass)
    free(map->rows_ger_second_pass);
    
  pthread_mutex_destroy(&map->mtx_rabbit_deaths);
  pthread_mutex_destroy(&map->mtx_fox_deaths);
  pthread_mutex_destroy(&map->mtx_rabbit_reprod);
  pthread_mutex_destroy(&map->mtx_fox_reprod);
  
  pthread_mutex_destroy(&map->mutex);
  pthread_cond_destroy(&map->cond);
    
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
  
  /* map initialization */
  map->ger_proc_coelhos = ger_proc_coelhos;
  map->ger_proc_raposas = ger_proc_raposas;
  map->ger_alim_raposas = ger_alim_raposas;
  map->n_ger = n_ger;
  map->lin = lin;
  map->col = col;
  map->rabbit_reprod = 0;
  map->fox_reprod = 0;
  map->rabbit_deaths = 0;
  map->fox_deaths = 0;
  pthread_mutex_init(&map->mtx_rabbit_deaths, NULL);
  pthread_mutex_init(&map->mtx_fox_deaths, NULL);
  pthread_mutex_init(&map->mtx_rabbit_reprod, NULL);
  pthread_mutex_init(&map->mtx_fox_reprod, NULL);
  
  map->next_row_first_pass = 0;
  map->next_row_second_pass = 0;
  map->rows_ger_first_pass = (int*)malloc(sizeof(int)*map->lin);
  map->rows_ger_second_pass = (int*)malloc(sizeof(int)*map->lin);
  
  for(i = 0; i < map->lin; ++i) {
    map->rows_ger_first_pass[i] = 0;
    map->rows_ger_second_pass[i] = 0;
  }
  
  map->proceed_first_pass = TRUE;
  map->proceed_second_pass = FALSE;
  map->the_end = FALSE;
  
  pthread_mutex_init(&map->mutex, NULL);
  pthread_cond_init(&map->cond, NULL);
  
  map->matrix = (Position*)malloc(sizeof(Position) * lin * col);
  
  for(i = 0; i < lin; ++i)
    for(j = 0; j < col; ++j)
      position_init(map_position_at(map, i, j));
  /* end of map initialization */
  
  char type[7];
  int x, y;
  Object* obj;
  Position* pos;
  
  for(i = 0; i < n_objs; ++i) {
    if(fscanf(fp, "%6s %d %d", type, &x, &y) != 3) {
      map_free(map);
      return NULL;
    }
    
    printf("Read %s %d %d\n", type, x, y);
    
    if(!map_inside(map, x, y)) {
      map_free(map);
      return NULL;
    }
     
    pos = map_position_at(map, x, y); 
    
    if(strcmp(type, "ROCHA") == 0) {
      pos->is_rock = TRUE;
      continue;
    }
    
    if(strcmp(type, "RAPOSA") == 0)
      obj = (Object*)object_new_fox(x, y);
    else if(strcmp(type, "COELHO") == 0)
      obj = (Object*)object_new_rabbit(x, y);
    else
      obj = NULL;
      
    if(!obj) {
      map_free(map);
      return NULL;
    }
    
    pos->obj = obj;
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
      Position *pos = map_position_at(map, i, j);
      
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
map_statistics(Map* map)
{
  int total_rabbits, total_foxes;
  int i, j;
  
  total_rabbits = total_foxes = 0;
  
  for(i = 0; i < map->lin; ++i)
    for(j = 0; j < map->col; ++j) {
      Position* pos = map_position_at(map, i, j);
      
      if(pos->obj) {
        switch(pos->obj->type) {
          case RABBIT: ++total_rabbits; break;
          case FOX: ++total_foxes; break;
        }
      }
    }
    
  printf("===================================\n");
  printf("  || FOXES: %d\tRABBITS: %d\n", total_foxes, total_rabbits);
  printf("  || RABBIT: +%d  -%d\n", map->rabbit_reprod, map->rabbit_deaths);
  printf("  || FOX: %d  -%d\n", map->fox_reprod, map->fox_deaths);
  printf("===================================\n");
}

void
map_output(Map* map, FILE* fp)
{
  int i, j;
  
  fprintf(fp, "MATRIZ %d %d\n", map->lin, map->col);
  
  for(i = 0; i < map->lin; ++i) {
    for(j = 0; j < map->col; ++j) {
      Position* pos = map_position_at(map, i, j);
      
      if(pos->is_rock)
        fprintf(fp, "ROCHA %d %d\n", i, j);
      else if(pos->obj)
        switch(pos->obj->type) {
          case FOX: fprintf(fp, "RAPOSA %d %d\n", i, j); break;
          case RABBIT: fprintf(fp, "COELHO %d %d\n", i, j); break;
        }
    }
  }
}

Boolean
map_inside_coord(Map* map, Coord coord)
{
  return map_inside(map, Coord_x(coord), Coord_y(coord));
}

Boolean
map_inside(Map* map, int x, int y)
{
  return x >= 0 &&
         y >= 0 &&
         x < map->lin &&
         y < map->col;
}

int
map_next_row(Map* map, int row)
{
  return (row + 1) % map->lin;
}