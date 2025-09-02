/**
 * @file map_impl.c
 * @brief Implementation of Map with MapNode integration for zero-cognitive-load usage
 */

#include "map.h"
#include "mapnode.h"
#include "mapnode_impl.c"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ======================================================================== */
/* Private Map Structure                                                    */
/* ======================================================================== */

typedef struct MapEntry {
    void* key;                    /* MapNode key */
    void* value;                  /* MapNode value */
    struct MapEntry* next;        /* Next entry in collision chain */
} MapEntry;

typedef struct MapPrivate {
    Map public;                   /* Public interface MUST be first */
    MapEntry** buckets;           /* Array of bucket heads */
    size_t capacity;              /* Number of buckets */
    size_t size;                  /* Number of entries */
    float max_load_factor;        /* Resize threshold */
} MapPrivate;

/* ======================================================================== */
/* Utility Functions                                                        */
/* ======================================================================== */

static size_t next_power_of_2(size_t n) {
    if (n <= 1) return 2;
    if ((n & (n - 1)) == 0) return n;
    
    size_t result = 1;
    while (result < n) {
        result <<= 1;
    }
    return result;
}

static MapEntry* map_entry_create(void* key, void* value) {
    MapEntry* entry = calloc(1, sizeof(MapEntry));
    if (!entry) return NULL;
    
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

static void map_entry_free(MapEntry* entry) {
    if (!entry) return;
    
    /* Free the MapNode key and value */
    MapNode_Free(entry->key);
    MapNode_Free(entry->value);
    free(entry);
}

static void map_entry_chain_free(MapEntry* head) {
    while (head) {
        MapEntry* next = head->next;
        map_entry_free(head);
        head = next;
    }
}

/* ======================================================================== */
/* Internal Map Operations                                                  */
/* ======================================================================== */

static MapEntry* map_find_entry(MapPrivate* priv, void* key, size_t* out_bucket) {
    if (!priv || !MapNode_IsValid(key)) return NULL;
    
    size_t hash = MapNode_Hash(key);
    size_t bucket = hash & (priv->capacity - 1);
    
    if (out_bucket) *out_bucket = bucket;
    
    MapEntry* current = priv->buckets[bucket];
    while (current) {
        if (MapNode_Compare(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

static bool map_resize_internal(MapPrivate* priv, size_t new_capacity) {
    new_capacity = next_power_of_2(new_capacity);
    if (new_capacity == priv->capacity) return true;
    
    MapEntry** old_buckets = priv->buckets;
    size_t old_capacity = priv->capacity;
    
    priv->buckets = calloc(new_capacity, sizeof(MapEntry*));
    if (!priv->buckets) {
        priv->buckets = old_buckets;
        return false;
    }
    
    priv->capacity = new_capacity;
    priv->size = 0;
    
    /* Rehash all entries */
    {
        size_t i;
        for (i = 0; i < old_capacity; i++) {
        MapEntry* current = old_buckets[i];
        while (current) {
            MapEntry* next = current->next;
            
            /* Rehash this entry */
            size_t hash = MapNode_Hash(current->key);
            size_t bucket = hash & (new_capacity - 1);
            
            current->next = priv->buckets[bucket];
            priv->buckets[bucket] = current;
            priv->size++;
            
            current = next;
        }
    }
    }
    
    free(old_buckets);
    return true;
}

static void map_maybe_resize(MapPrivate* priv) {
    float load = (float)priv->size / (float)priv->capacity;
    if (load > priv->max_load_factor) {
        map_resize_internal(priv, priv->capacity * 2);
    }
}

/* ======================================================================== */
/* Map Trampoline Function Implementations                                 */
/* ======================================================================== */

bool map_put(Map* self, void* key, void* value) {
    MapPrivate* priv = (MapPrivate*)self;
    if (!priv || !MapNode_IsValid(key) || !MapNode_IsValid(value)) {
        return false;
    }
    
    size_t bucket;
    MapEntry* existing = map_find_entry(priv, key, &bucket);
    
    if (existing) {
        /* Update existing entry - free old value, store new one */
        MapNode_Free(existing->value);
        existing->value = value;
        return true;
    }
    
    /* Create new entry */
    MapEntry* entry = map_entry_create(key, value);
    if (!entry) return false;
    
    /* Insert at head of bucket chain */
    entry->next = priv->buckets[bucket];
    priv->buckets[bucket] = entry;
    priv->size++;
    
    map_maybe_resize(priv);
    return true;
}

void* map_get(Map* self, void* key) {
    MapPrivate* priv = (MapPrivate*)self;
    if (!priv || !MapNode_IsValid(key)) return NULL;
    
    MapEntry* entry = map_find_entry(priv, key, NULL);
    return entry ? entry->value : NULL;
}

bool map_remove(Map* self, void* key) {
    MapPrivate* priv = (MapPrivate*)self;
    if (!priv || !MapNode_IsValid(key)) return false;
    
    size_t hash = MapNode_Hash(key);
    size_t bucket = hash & (priv->capacity - 1);
    
    MapEntry** current = &priv->buckets[bucket];
    while (*current) {
        if (MapNode_Compare((*current)->key, key) == 0) {
            MapEntry* to_remove = *current;
            *current = to_remove->next;
            map_entry_free(to_remove);
            priv->size--;
            return true;
        }
        current = &(*current)->next;
    }
    
    return false;
}

bool map_contains(Map* self, void* key) {
    MapPrivate* priv = (MapPrivate*)self;
    return map_find_entry(priv, key, NULL) != NULL;
}

/* ======================================================================== */
/* Convenience Functions                                                    */
/* ======================================================================== */

bool map_put_int(Map* self, void* key, int value) {
    if (!MapNode_IsValid(key)) return false;
    
    void* value_node = MapNodeFromInt(value);
    if (!value_node) return false;
    
    return self->put(key, value_node);
}

bool map_put_float(Map* self, void* key, float value) {
    if (!MapNode_IsValid(key)) return false;
    
    void* value_node = MapNodeFromFloat(value);
    if (!value_node) return false;
    
    return self->put(key, value_node);
}

bool map_put_double(Map* self, void* key, double value) {
    if (!MapNode_IsValid(key)) return false;
    
    void* value_node = MapNodeFromDouble(value);
    if (!value_node) return false;
    
    return self->put(key, value_node);
}

bool map_put_string(Map* self, void* key, const char* value) {
    if (!MapNode_IsValid(key) || !value) return false;
    
    void* value_node = MapNodeFromString(value);
    if (!value_node) return false;
    
    return self->put(key, value_node);
}

bool map_put_pointer(Map* self, void* key, void* value) {
    if (!MapNode_IsValid(key)) return false;
    
    void* value_node = MapNodeFromPointer(value);
    if (!value_node) return false;
    
    return self->put(key, value_node);
}

int map_get_int(Map* self, void* key, int default_value) {
    void* value = self->get(key);
    if (!MapNode_IsValid(value)) return default_value;
    
    MapNode* node = MapNode_Cast(value);
    return node && node->isInt() ? node->asInt() : default_value;
}

float map_get_float(Map* self, void* key, float default_value) {
    void* value = self->get(key);
    if (!MapNode_IsValid(value)) return default_value;
    
    MapNode* node = MapNode_Cast(value);
    return node && node->isFloat() ? node->asFloat() : default_value;
}

double map_get_double(Map* self, void* key, double default_value) {
    void* value = self->get(key);
    if (!MapNode_IsValid(value)) return default_value;
    
    MapNode* node = MapNode_Cast(value);
    return node && node->isDouble() ? node->asDouble() : default_value;
}

const char* map_get_string(Map* self, void* key) {
    void* value = self->get(key);
    if (!MapNode_IsValid(value)) return NULL;
    
    MapNode* node = MapNode_Cast(value);
    return node && node->isString() ? node->asString() : NULL;
}

void* map_get_pointer(Map* self, void* key) {
    void* value = self->get(key);
    if (!MapNode_IsValid(value)) return NULL;
    
    MapNode* node = MapNode_Cast(value);
    return node && node->isPointer() ? node->asPointer() : NULL;
}

/* ======================================================================== */
/* Map Information Functions                                                */
/* ======================================================================== */

size_t map_size(Map* self) {
    MapPrivate* priv = (MapPrivate*)self;
    return priv ? priv->size : 0;
}

bool map_is_empty(Map* self) {
    MapPrivate* priv = (MapPrivate*)self;
    return priv ? (priv->size == 0) : true;
}

size_t map_capacity(Map* self) {
    MapPrivate* priv = (MapPrivate*)self;
    return priv ? priv->capacity : 0;
}

float map_load_factor(Map* self) {
    MapPrivate* priv = (MapPrivate*)self;
    return priv ? ((float)priv->size / (float)priv->capacity) : 0.0f;
}

void map_clear(Map* self) {
    MapPrivate* priv = (MapPrivate*)self;
    size_t i;
    
    if (!priv) return;
    
    for (i = 0; i < priv->capacity; i++) {
        map_entry_chain_free(priv->buckets[i]);
        priv->buckets[i] = NULL;
    }
    
    priv->size = 0;
}

void map_resize(Map* self, size_t new_capacity) {
    MapPrivate* priv = (MapPrivate*)self;
    if (!priv) return;
    
    map_resize_internal(priv, new_capacity);
}

void** map_get_all_keys(Map* self, size_t* out_count) {
    MapPrivate* priv = (MapPrivate*)self;
    if (!priv || !out_count) {
        if (out_count) *out_count = 0;
        return NULL;
    }
    
    if (priv->size == 0) {
        *out_count = 0;
        return NULL;
    }
    
    void** keys = malloc(sizeof(void*) * priv->size);
    if (!keys) {
        *out_count = 0;
        return NULL;
    }
    
    {
        size_t index = 0;
        size_t i;
        for (i = 0; i < priv->capacity; i++) {
            MapEntry* current = priv->buckets[i];
            while (current && index < priv->size) {
                keys[index++] = current->key;
                current = current->next;
            }
        }
        
        *out_count = index;
        return keys;
    }
}

void** map_get_all_values(Map* self, size_t* out_count) {
    MapPrivate* priv = (MapPrivate*)self;
    if (!priv || !out_count) {
        if (out_count) *out_count = 0;
        return NULL;
    }
    
    if (priv->size == 0) {
        *out_count = 0;
        return NULL;
    }
    
    {
        void** values = malloc(sizeof(void*) * priv->size);
        size_t index = 0;
        size_t i;
        
        if (!values) {
            *out_count = 0;
            return NULL;
        }
        
        for (i = 0; i < priv->capacity; i++) {
            MapEntry* current = priv->buckets[i];
            while (current && index < priv->size) {
                values[index++] = current->value;
                current = current->next;
            }
        }
        
        *out_count = index;
        return values;
    }
}

/* ======================================================================== */
/* Debugging Functions                                                      */
/* ======================================================================== */

void map_debug(Map* self, size_t max_entries) {
    MapPrivate* priv = (MapPrivate*)self;
    if (!priv) {
        printf("Map: NULL\n");
        return;
    }
    
    printf("Map Debug Info:\n");
    printf("  Size: %zu, Capacity: %zu, Load Factor: %.2f\n", 
           priv->size, priv->capacity, map_load_factor(self));
    
    if (max_entries == 0) max_entries = priv->size;
    
    {
        size_t printed = 0;
        size_t i;
        for (i = 0; i < priv->capacity && printed < max_entries; i++) {
        MapEntry* current = priv->buckets[i];
        size_t chain_length = 0;
        
        while (current && printed < max_entries) {
            char key_str[128], value_str[128];
            MapNode_ToString(current->key, key_str, sizeof(key_str));
            MapNode_ToString(current->value, value_str, sizeof(value_str));
            
            printf("  [%zu:%zu] %s -> %s\n", i, chain_length, key_str, value_str);
            
            current = current->next;
            chain_length++;
            printed++;
        }
        }
        
        if (printed < priv->size) {
            printf("  ... (%zu more entries)\n", priv->size - printed);
        }
    }
}

size_t map_validate(Map* self) {
    MapPrivate* priv = (MapPrivate*)self;
    if (!priv) {
        fprintf(stderr, "Map validation: NULL map\n");
        return 1;
    }
    
    {
        size_t errors = 0;
        size_t actual_size = 0;
        size_t i;
        
        for (i = 0; i < priv->capacity; i++) {
        MapEntry* current = priv->buckets[i];
        while (current) {
            actual_size++;
            
            if (!MapNode_IsValid(current->key)) {
                fprintf(stderr, "Map validation: Invalid key in bucket %zu\n", i);
                errors++;
            }
            
            if (!MapNode_IsValid(current->value)) {
                fprintf(stderr, "Map validation: Invalid value in bucket %zu\n", i);
                errors++;
            }
            
            current = current->next;
        }
        }
        
        if (actual_size != priv->size) {
            fprintf(stderr, "Map validation: Size mismatch (stored: %zu, actual: %zu)\n", 
                    priv->size, actual_size);
            errors++;
        }
        
        return errors;
    }
}

bool map_get_stats(Map* self, void* stats_ptr) {
    MapPrivate* priv = (MapPrivate*)self;
    struct MapStats* stats = (struct MapStats*)stats_ptr;
    if (!priv || !stats) return false;
    
    memset(stats, 0, sizeof(struct MapStats));
    
    stats->entry_count = priv->size;
    stats->bucket_count = priv->capacity;
    stats->load_factor = map_load_factor(self);
    
    {
        size_t empty_buckets = 0;
        size_t max_chain = 0;
        size_t total_chain_length = 0;
        size_t i;
        
        /* Analyze buckets and collect type statistics */
        for (i = 0; i < priv->capacity; i++) {
            MapEntry* current = priv->buckets[i];
            size_t chain_length = 0;
        
        if (!current) {
            empty_buckets++;
        } else {
            while (current) {
                chain_length++;
                total_chain_length++;
                
                /* Count key types */
                MapNode* key_node = MapNode_Cast(current->key);
                if (key_node) {
                    switch (key_node->type()) {
                        case MAPNODE_TYPE_INT: stats->int_keys++; break;
                        case MAPNODE_TYPE_FLOAT: stats->float_keys++; break;
                        case MAPNODE_TYPE_DOUBLE: stats->double_keys++; break;
                        case MAPNODE_TYPE_STRING: stats->string_keys++; break;
                        case MAPNODE_TYPE_POINTER: stats->pointer_keys++; break;
                        case MAPNODE_TYPE_BYTES: stats->bytes_keys++; break;
                    }
                }
                
                /* Count value types */
                MapNode* value_node = MapNode_Cast(current->value);
                if (value_node) {
                    switch (value_node->type()) {
                        case MAPNODE_TYPE_INT: stats->int_values++; break;
                        case MAPNODE_TYPE_FLOAT: stats->float_values++; break;
                        case MAPNODE_TYPE_DOUBLE: stats->double_values++; break;
                        case MAPNODE_TYPE_STRING: stats->string_values++; break;
                        case MAPNODE_TYPE_POINTER: stats->pointer_values++; break;
                        case MAPNODE_TYPE_BYTES: stats->bytes_values++; break;
                    }
                }
                
                current = current->next;
            }
            
            if (chain_length > max_chain) {
                max_chain = chain_length;
            }
        }
    }
    
    stats->empty_buckets = empty_buckets;
        stats->max_chain_length = max_chain;
        stats->average_chain_length = priv->size > 0 ? 
            (float)total_chain_length / (float)(priv->capacity - empty_buckets) : 0.0f;
        
        /* Rough memory estimate */
        stats->total_memory = sizeof(MapPrivate) + 
                             (priv->capacity * sizeof(MapEntry*)) +
                             (priv->size * sizeof(MapEntry));
        
        return true;
    }
}

void map_free(Map* self) {
    MapPrivate* priv = (MapPrivate*)self;
    if (!priv) return;
    
    /* Free all entries */
    map_clear(self);
    
    /* Free bucket array */
    free(priv->buckets);
    
    /* Free trampoline functions */
    if (self->put) trampoline_free(self->put);
    if (self->get) trampoline_free(self->get);
    if (self->remove) trampoline_free(self->remove);
    if (self->contains) trampoline_free(self->contains);
    if (self->putInt) trampoline_free(self->putInt);
    if (self->putFloat) trampoline_free(self->putFloat);
    if (self->putDouble) trampoline_free(self->putDouble);
    if (self->putString) trampoline_free(self->putString);
    if (self->putPointer) trampoline_free(self->putPointer);
    if (self->getInt) trampoline_free(self->getInt);
    if (self->getFloat) trampoline_free(self->getFloat);
    if (self->getDouble) trampoline_free(self->getDouble);
    if (self->getString) trampoline_free(self->getString);
    if (self->getPointer) trampoline_free(self->getPointer);
    if (self->size) trampoline_free(self->size);
    if (self->isEmpty) trampoline_free(self->isEmpty);
    if (self->capacity) trampoline_free(self->capacity);
    if (self->loadFactor) trampoline_free(self->loadFactor);
    if (self->clear) trampoline_free(self->clear);
    if (self->resize) trampoline_free(self->resize);
    if (self->getAllKeys) trampoline_free(self->getAllKeys);
    if (self->getAllValues) trampoline_free(self->getAllValues);
    if (self->debug) trampoline_free(self->debug);
    if (self->validate) trampoline_free(self->validate);
    if (self->getStats) trampoline_free(self->getStats);
    if (self->free) trampoline_free(self->free);
    
    /* Free the map structure itself */
    free(priv);
}

/* ======================================================================== */
/* Map Creation Functions                                                   */
/* ======================================================================== */

static Map* map_make_internal(size_t initial_capacity) {
    initial_capacity = next_power_of_2(initial_capacity);
    if (initial_capacity < 4) initial_capacity = 4;
    
    MapPrivate* priv = calloc(1, sizeof(MapPrivate));
    if (!priv) return NULL;
    
    priv->buckets = calloc(initial_capacity, sizeof(MapEntry*));
    if (!priv->buckets) {
        free(priv);
        return NULL;
    }
    
    priv->capacity = initial_capacity;
    priv->size = 0;
    priv->max_load_factor = 0.75f;
    
    /* Get reference to embedded public interface */
    Map* map = &priv->public;
    
    /* Create trampoline functions */
    trampoline_allocations allocations = {0};
    
    /* Core operations */
    map->put = trampoline_create_and_track(map_put, map, 2, &allocations);
    map->get = trampoline_create_and_track(map_get, map, 1, &allocations);
    map->remove = trampoline_create_and_track(map_remove, map, 1, &allocations);
    map->contains = trampoline_create_and_track(map_contains, map, 1, &allocations);
    
    /* Convenience functions */
    map->putInt = trampoline_create_and_track(map_put_int, map, 2, &allocations);
    map->putFloat = trampoline_create_and_track(map_put_float, map, 2, &allocations);
    map->putDouble = trampoline_create_and_track(map_put_double, map, 2, &allocations);
    map->putString = trampoline_create_and_track(map_put_string, map, 2, &allocations);
    map->putPointer = trampoline_create_and_track(map_put_pointer, map, 2, &allocations);
    map->getInt = trampoline_create_and_track(map_get_int, map, 2, &allocations);
    map->getFloat = trampoline_create_and_track(map_get_float, map, 2, &allocations);
    map->getDouble = trampoline_create_and_track(map_get_double, map, 2, &allocations);
    map->getString = trampoline_create_and_track(map_get_string, map, 1, &allocations);
    map->getPointer = trampoline_create_and_track(map_get_pointer, map, 1, &allocations);
    
    /* Information functions */
    map->size = trampoline_create_and_track(map_size, map, 0, &allocations);
    map->isEmpty = trampoline_create_and_track(map_is_empty, map, 0, &allocations);
    map->capacity = trampoline_create_and_track(map_capacity, map, 0, &allocations);
    map->loadFactor = trampoline_create_and_track(map_load_factor, map, 0, &allocations);
    map->clear = trampoline_create_and_track(map_clear, map, 0, &allocations);
    map->resize = trampoline_create_and_track(map_resize, map, 1, &allocations);
    
    /* Bulk operations */
    map->getAllKeys = trampoline_create_and_track(map_get_all_keys, map, 1, &allocations);
    map->getAllValues = trampoline_create_and_track(map_get_all_values, map, 1, &allocations);
    
    /* Debug functions */
    map->debug = trampoline_create_and_track(map_debug, map, 1, &allocations);
    map->validate = trampoline_create_and_track(map_validate, map, 0, &allocations);
    map->getStats = trampoline_create_and_track(map_get_stats, map, 1, &allocations);
    
    /* Management */
    map->free = trampoline_create_and_track(map_free, map, 0, &allocations);
    
    if (!trampolines_validate(&allocations)) {
        free(priv->buckets);
        free(priv);
        return NULL;
    }
    
    return map;
}

Map* MapMake(void) {
    return map_make_internal(16);
}

Map* MapMakeWithCapacity(size_t initial_capacity) {
    return map_make_internal(initial_capacity);
}

/* ======================================================================== */
/* Utility Functions                                                        */
/* ======================================================================== */

struct MapPair MapPair_Make(void* key, void* value) {
    struct MapPair pair;
    pair.key = (MapNode_IsValid(key)) ? key : NULL;
    pair.value = (MapNode_IsValid(value)) ? value : NULL;
    return pair;
}

size_t Map_PutAll(Map* map, struct MapPair* pairs, size_t count) {
    size_t inserted = 0;
    size_t i;
    
    if (!map || !pairs) return 0;
    
    for (i = 0; i < count; i++) {
        if (pairs[i].key && pairs[i].value) {
            if (map->put(pairs[i].key, pairs[i].value)) {
                inserted++;
            }
        }
    }
    
    return inserted;
}