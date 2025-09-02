#include <stdio.h>
#include <stdlib.h>
#include <trampoline.h>

// Minimal map structure for testing
typedef struct MapPrivate {
    size_t size;
} MapPrivate;

typedef struct Map {
    size_t (*size)();
    void (*free)();
} Map;

// Trampoline functions
size_t map_size(Map* self) {
    MapPrivate* priv = (MapPrivate*)self;
    return priv ? priv->size : 0;
}

void map_free(Map* self) {
    MapPrivate* priv = (MapPrivate*)self;
    if (!priv) return;
    
    if (self->size) trampoline_free(self->size);
    if (self->free) trampoline_free(self->free);
    
    free(priv);
}

Map* MapMake(void) {
    printf("DEBUG: Starting MapMake\n");
    
    MapPrivate* priv = calloc(1, sizeof(MapPrivate));
    if (!priv) {
        printf("DEBUG: Failed to allocate MapPrivate\n");
        return NULL;
    }
    
    printf("DEBUG: Allocated MapPrivate at %p\n", (void*)priv);
    priv->size = 0;
    
    Map* map = (Map*)priv;
    
    printf("DEBUG: Creating trampoline allocations\n");
    trampoline_allocations allocations = {0};
    
    printf("DEBUG: Creating size trampoline\n");
    map->size = trampoline_create_and_track(map_size, map, 0, &allocations);
    printf("DEBUG: Size trampoline: %p\n", (void*)map->size);
    
    printf("DEBUG: Creating free trampoline\n");  
    map->free = trampoline_create_and_track(map_free, map, 0, &allocations);
    printf("DEBUG: Free trampoline: %p\n", (void*)map->free);
    
    printf("DEBUG: Validating trampolines\n");
    if (!trampolines_validate(&allocations)) {
        printf("DEBUG: Trampoline validation failed\n");
        free(priv);
        return NULL;
    }
    
    printf("DEBUG: MapMake completed successfully\n");
    return map;
}

int main() {
    printf("Minimal Map Test\n");
    printf("================\n");
    
    Map* map = MapMake();
    if (!map) {
        printf("Failed to create map\n");
        return 1;
    }
    
    printf("Testing size: %zu\n", map->size());
    
    map->free();
    
    printf("Test completed!\n");
    return 0;
}