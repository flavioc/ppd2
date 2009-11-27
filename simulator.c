
#include <stdlib.h>

#include "map.h"

static Map* map = NULL;
static int nthreads = 4;

int
main()
{
  map = map_read(stdin);
  
  map_print(map);
  
  map_free(map);
  
  return EXIT_SUCCESS;
}
