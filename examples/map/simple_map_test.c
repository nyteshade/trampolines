#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../trampoline.h"

#include "mapnode.h"
#include "map.h"
#include "map_impl.c"

int main() {
    printf("Simple Map Test\n");
    printf("===============\n");
    
    printf("Creating map...\n");
    Map* map = MapMake();
    if (!map) {
        printf("Failed to create map\n");
        return 1;
    }
    
    printf("Map created successfully\n");
    printf("Initial capacity: %zu\n", map->capacity());
    printf("Initial size: %zu\n", map->size());
    
    printf("Creating MapNodes...\n");
    void* key = MapNodeFromString("test_key");
    void* value = MapNodeFromString("test_value");
    
    if (!MapNode_IsValid(key) || !MapNode_IsValid(value)) {
        printf("Failed to create MapNodes\n");
        return 1;
    }
    
    printf("MapNodes created successfully\n");
    
    printf("Inserting into map...\n");
    bool result = map->put(key, value);
    printf("Insert result: %s\n", result ? "success" : "failed");
    
    if (result) {
        printf("Map size after insert: %zu\n", map->size());
        
        printf("Retrieving from map...\n");
        void* retrieved = map->get(key);
        
        if (retrieved) {
            printf("Retrieved successfully\n");
            MapNode* node = MapNode_Cast(retrieved);
            if (node) {
                printf("Retrieved value: %s\n", node->asString());
            }
        } else {
            printf("Failed to retrieve\n");
        }
    }
    
    printf("Freeing map...\n");
    map->free();
    
    printf("Test completed!\n");
    return 0;
}