#include <stdio.h>
#include <stdlib.h>
#include "../../trampoline.h"

#include "mapnode.h"
#include "map.h"
#include "map_impl.c"

int main() {
    printf("DEBUG: Starting minimal map test\n");
    
    printf("DEBUG: About to call MapMake()\n");
    Map* map = MapMake();
    printf("DEBUG: MapMake() returned: %p\n", (void*)map);
    
    if (!map) {
        printf("DEBUG: MapMake() failed\n");
        return 1;
    }
    
    printf("DEBUG: Map creation succeeded\n");
    printf("DEBUG: About to call map->capacity()\n");
    size_t cap = map->capacity();
    printf("DEBUG: Capacity: %zu\n", cap);
    
    printf("DEBUG: About to call map->size()\n");
    size_t sz = map->size();
    printf("DEBUG: Size: %zu\n", sz);
    
    printf("DEBUG: About to call map->free()\n");
    map->free();
    printf("DEBUG: Free completed\n");
    
    printf("DEBUG: Test completed successfully\n");
    return 0;
}