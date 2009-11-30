
#include <stdlib.h>
#include <stdio.h>

#include "map.h"

int
main(int argc, char **argv)
{
  if(argc != 8) {
    fprintf(stderr, "usage: generator <lin> <col> <ger_proc_coelhos> <ger_proc_raposas> <ger_alim_raposas> <n_ger> <density>\n");
    return EXIT_FAILURE;
  }
  
  int lin = atoi(argv[1]);
  int col = atoi(argv[2]);
  int ger_proc_coelhos = atoi(argv[3]);
  int ger_proc_raposas = atoi(argv[4]);
  int ger_alim_raposas = atoi(argv[5]);
  int n_ger = atoi(argv[6]);
  int density = atoi(argv[7]);
  
  Map* map = map_generate(lin, col, ger_proc_coelhos, ger_proc_raposas, ger_alim_raposas, n_ger, density);
  //map_print(map);
  map_write(map, stdout);
  map_free(map);
  
  return EXIT_SUCCESS;
}