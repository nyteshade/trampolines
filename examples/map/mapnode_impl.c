/**
 * @file mapnode_impl.c
 * @brief Implementation of MapNode with magic byte introspection
 */

#include "mapnode.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ======================================================================== */
/* Private MapNode Structure                                                */
/* ======================================================================== */

/**
 * @brief Private implementation of MapNode
 * 
 * Memory layout is crucial for magic byte introspection:
 * - data pointer at offset 0 (this is what gets passed around as void*)  
 * - magic bytes immediately after data pointer for validation
 * - type information follows magic bytes
 * - public interface embedded later in the struct
 */
typedef struct MagicMapNode {
    void* data;              /* Offset 0 - the pointer passed around */
    uint32_t magic;          /* Offset sizeof(void*) - magic bytes */
    uint32_t type;           /* Offset sizeof(void*) + 4 - type enum */
    size_t data_size;        /* Size of data pointed to by data pointer */
    bool owns_data;          /* Whether we need to free the data */
    
    MapNode public;          /* Public trampoline interface */
    
    /* Additional private fields can go here */
} MagicMapNode;

/* ======================================================================== */
/* Helper Macros for Trampoline Functions                                  */
/* ======================================================================== */

/**
 * @brief Get private struct from public MapNode interface
 * Since public is embedded in the private struct at a specific offset,
 * we calculate backwards to get the container struct.
 */
#define MAPNODE_PRIVATE(public_ptr) \
    ((MagicMapNode*)((char*)(public_ptr) - offsetof(MagicMapNode, public)))

/**
 * @brief Create trampoline getter that returns a field from private struct
 */
#define MAPNODE_GETTER(name, return_type, expression) \
    return_type mapnode_##name(MapNode* self) { \
        MagicMapNode* priv = MAPNODE_PRIVATE(self); \
        return (expression); \
    }

/**
 * @brief Create trampoline type checker
 */
#define MAPNODE_TYPE_CHECKER(name, magic_const, type_enum) \
    bool mapnode_is_##name(MapNode* self) { \
        MagicMapNode* priv = MAPNODE_PRIVATE(self); \
        return priv->magic == (magic_const) && priv->type == (type_enum); \
    }

/* ======================================================================== */
/* Introspection Functions                                                  */
/* ======================================================================== */

bool MapNode_IsValid(const void* ptr) {
    if (!ptr) return false;
    
    /* Calculate where magic bytes should be */
    const char* byte_ptr = (const char*)ptr;
    const uint32_t* magic_ptr = (const uint32_t*)(byte_ptr + sizeof(void*));
    
    /* Check if magic bytes match any of our known values */
    uint32_t magic = *magic_ptr;
    return (magic == MAPNODE_MAGIC_INT ||
            magic == MAPNODE_MAGIC_FLOAT ||
            magic == MAPNODE_MAGIC_DOUBLE ||
            magic == MAPNODE_MAGIC_STRING ||
            magic == MAPNODE_MAGIC_POINTER ||
            magic == MAPNODE_MAGIC_BYTES);
}

MapNodeType MapNode_GetType(const void* ptr) {
    if (!MapNode_IsValid(ptr)) return 0;
    
    const char* byte_ptr = (const char*)ptr;
    const uint32_t* type_ptr = (const uint32_t*)(byte_ptr + sizeof(void*) + sizeof(uint32_t));
    
    return (MapNodeType)(*type_ptr);
}

uint32_t MapNode_GetMagic(const void* ptr) {
    if (!ptr) return 0;
    
    const char* byte_ptr = (const char*)ptr;
    const uint32_t* magic_ptr = (const uint32_t*)(byte_ptr + sizeof(void*));
    
    return *magic_ptr;
}

MapNode* MapNode_Cast(void* ptr) {
    if (!MapNode_IsValid(ptr)) return NULL;
    
    /* ptr points to MagicMapNode.data, we need to get to MagicMapNode.public */
    MagicMapNode* priv = (MagicMapNode*)ptr;
    return &priv->public;
}

size_t MapNode_GetSize(const void* ptr) {
    if (!MapNode_IsValid(ptr)) return 0;
    
    const MagicMapNode* priv = (const MagicMapNode*)ptr;
    return priv->data_size;
}

/* ======================================================================== */
/* Trampoline Function Implementations                                     */
/* ======================================================================== */

/* Type conversion functions */
MAPNODE_GETTER(as_int, int, 
    (priv->type == MAPNODE_TYPE_INT && priv->data) ? *(int*)priv->data : 0)

MAPNODE_GETTER(as_float, float,
    (priv->type == MAPNODE_TYPE_FLOAT && priv->data) ? *(float*)priv->data : 0.0f)

MAPNODE_GETTER(as_double, double,
    (priv->type == MAPNODE_TYPE_DOUBLE && priv->data) ? *(double*)priv->data : 0.0)

MAPNODE_GETTER(as_string, const char*,
    (priv->type == MAPNODE_TYPE_STRING) ? (const char*)priv->data : NULL)

MAPNODE_GETTER(as_pointer, void*,
    (priv->type == MAPNODE_TYPE_POINTER && priv->data) ? *(void**)priv->data : NULL)

const void* mapnode_as_bytes(MapNode* self, size_t* out_size) {
    MagicMapNode* priv = MAPNODE_PRIVATE(self);
    if (out_size) *out_size = priv->data_size;
    return priv->data;
}

/* Type checking functions */
MAPNODE_TYPE_CHECKER(int, MAPNODE_MAGIC_INT, MAPNODE_TYPE_INT)
MAPNODE_TYPE_CHECKER(float, MAPNODE_MAGIC_FLOAT, MAPNODE_TYPE_FLOAT)
MAPNODE_TYPE_CHECKER(double, MAPNODE_MAGIC_DOUBLE, MAPNODE_TYPE_DOUBLE)
MAPNODE_TYPE_CHECKER(string, MAPNODE_MAGIC_STRING, MAPNODE_TYPE_STRING)
MAPNODE_TYPE_CHECKER(pointer, MAPNODE_MAGIC_POINTER, MAPNODE_TYPE_POINTER)
MAPNODE_TYPE_CHECKER(bytes, MAPNODE_MAGIC_BYTES, MAPNODE_TYPE_BYTES)

/* Utility functions */
MAPNODE_GETTER(size, size_t, priv->data_size)
MAPNODE_GETTER(type, MapNodeType, (MapNodeType)priv->type)

const char* mapnode_type_name(MapNode* self) {
    MagicMapNode* priv = MAPNODE_PRIVATE(self);
    switch (priv->type) {
        case MAPNODE_TYPE_INT: return "int";
        case MAPNODE_TYPE_FLOAT: return "float";
        case MAPNODE_TYPE_DOUBLE: return "double";
        case MAPNODE_TYPE_STRING: return "string";
        case MAPNODE_TYPE_POINTER: return "pointer";
        case MAPNODE_TYPE_BYTES: return "bytes";
        default: return "unknown";
    }
}

void* mapnode_copy(MapNode* self) {
    MagicMapNode* priv = MAPNODE_PRIVATE(self);
    
    /* Delegate to the appropriate constructor based on type */
    switch (priv->type) {
        case MAPNODE_TYPE_INT:
            return MapNodeFromInt(*(int*)priv->data);
        case MAPNODE_TYPE_FLOAT:
            return MapNodeFromFloat(*(float*)priv->data);
        case MAPNODE_TYPE_DOUBLE:
            return MapNodeFromDouble(*(double*)priv->data);
        case MAPNODE_TYPE_STRING:
            return MapNodeFromString((const char*)priv->data);
        case MAPNODE_TYPE_POINTER:
            return MapNodeFromPointer(*(void**)priv->data);
        case MAPNODE_TYPE_BYTES:
            return MapNodeFromBytes(priv->data, priv->data_size);
        default:
            return NULL;
    }
}

void mapnode_free(MapNode* self) {
    MagicMapNode* priv = MAPNODE_PRIVATE(self);
    
    /* Free the data if we own it */
    if (priv->owns_data && priv->data) {
        free(priv->data);
    }
    
    /* Free trampoline functions */
    if (self->asInt) trampoline_free(self->asInt);
    if (self->asFloat) trampoline_free(self->asFloat);
    if (self->asDouble) trampoline_free(self->asDouble);
    if (self->asString) trampoline_free(self->asString);
    if (self->asPointer) trampoline_free(self->asPointer);
    if (self->asBytes) trampoline_free(self->asBytes);
    if (self->isInt) trampoline_free(self->isInt);
    if (self->isFloat) trampoline_free(self->isFloat);
    if (self->isDouble) trampoline_free(self->isDouble);
    if (self->isString) trampoline_free(self->isString);
    if (self->isPointer) trampoline_free(self->isPointer);
    if (self->isBytes) trampoline_free(self->isBytes);
    if (self->typeName) trampoline_free(self->typeName);
    if (self->size) trampoline_free(self->size);
    if (self->type) trampoline_free(self->type);
    if (self->copy) trampoline_free(self->copy);
    if (self->free) trampoline_free(self->free);
    
    /* Clear magic bytes to make debugging easier */
    priv->magic = 0xDEADBEEF;
    
    /* Free the MapNode structure itself */
    free(priv);
}

/* ======================================================================== */
/* Internal Constructor Helper                                              */
/* ======================================================================== */

/**
 * @brief Internal function to create MapNode with given parameters
 */
static void* mapnode_create_internal(uint32_t magic, MapNodeType type, 
                                     const void* data, size_t data_size, 
                                     bool copy_data) {
    MagicMapNode* node = calloc(1, sizeof(MagicMapNode));
    if (!node) return NULL;
    
    /* Set up the magic bytes and type for introspection */
    node->magic = magic;
    node->type = type;
    node->data_size = data_size;
    node->owns_data = copy_data;
    
    /* Handle data storage */
    if (copy_data && data && data_size > 0) {
        node->data = malloc(data_size);
        if (!node->data) {
            free(node);
            return NULL;
        }
        memcpy(node->data, data, data_size);
    } else {
        /* Just store the pointer (for non-copied data like string literals) */
        node->data = (void*)data;
    }
    
    /* Set up the trampoline functions */
    trampoline_allocations allocations = {0};
    
    node->public.asInt = trampoline_create_and_track(mapnode_as_int, &node->public, 0, &allocations);
    node->public.asFloat = trampoline_create_and_track(mapnode_as_float, &node->public, 0, &allocations);
    node->public.asDouble = trampoline_create_and_track(mapnode_as_double, &node->public, 0, &allocations);
    node->public.asString = trampoline_create_and_track(mapnode_as_string, &node->public, 0, &allocations);
    node->public.asPointer = trampoline_create_and_track(mapnode_as_pointer, &node->public, 0, &allocations);
    node->public.asBytes = trampoline_create_and_track(mapnode_as_bytes, &node->public, 1, &allocations);
    
    node->public.isInt = trampoline_create_and_track(mapnode_is_int, &node->public, 0, &allocations);
    node->public.isFloat = trampoline_create_and_track(mapnode_is_float, &node->public, 0, &allocations);
    node->public.isDouble = trampoline_create_and_track(mapnode_is_double, &node->public, 0, &allocations);
    node->public.isString = trampoline_create_and_track(mapnode_is_string, &node->public, 0, &allocations);
    node->public.isPointer = trampoline_create_and_track(mapnode_is_pointer, &node->public, 0, &allocations);
    node->public.isBytes = trampoline_create_and_track(mapnode_is_bytes, &node->public, 0, &allocations);
    
    node->public.typeName = trampoline_create_and_track(mapnode_type_name, &node->public, 0, &allocations);
    node->public.size = trampoline_create_and_track(mapnode_size, &node->public, 0, &allocations);
    node->public.type = trampoline_create_and_track(mapnode_type, &node->public, 0, &allocations);
    node->public.copy = trampoline_create_and_track(mapnode_copy, &node->public, 0, &allocations);
    node->public.free = trampoline_create_and_track(mapnode_free, &node->public, 0, &allocations);
    
    /* Validate that all trampolines were created successfully */
    if (!trampolines_validate(&allocations)) {
        if (node->owns_data && node->data) {
            free(node->data);
        }
        free(node);
        return NULL;
    }
    
    /* Return pointer to data member (offset 0), which is what gets passed around */
    return &node->data;
}

/* ======================================================================== */
/* Constructor Functions                                                    */
/* ======================================================================== */

void* MapNodeFromInt(int value) {
    return mapnode_create_internal(MAPNODE_MAGIC_INT, MAPNODE_TYPE_INT, 
                                   &value, sizeof(int), true);
}

void* MapNodeFromFloat(float value) {
    return mapnode_create_internal(MAPNODE_MAGIC_FLOAT, MAPNODE_TYPE_FLOAT, 
                                   &value, sizeof(float), true);
}

void* MapNodeFromDouble(double value) {
    return mapnode_create_internal(MAPNODE_MAGIC_DOUBLE, MAPNODE_TYPE_DOUBLE, 
                                   &value, sizeof(double), true);
}

void* MapNodeFromString(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str) + 1;  /* Include null terminator */
    return mapnode_create_internal(MAPNODE_MAGIC_STRING, MAPNODE_TYPE_STRING, 
                                   str, len, true);
}

void* MapNodeFromPointer(void* ptr) {
    return mapnode_create_internal(MAPNODE_MAGIC_POINTER, MAPNODE_TYPE_POINTER, 
                                   &ptr, sizeof(void*), true);
}

void* MapNodeFromBytes(const void* data, size_t size) {
    if (!data || size == 0) return NULL;
    
    return mapnode_create_internal(MAPNODE_MAGIC_BYTES, MAPNODE_TYPE_BYTES, 
                                   data, size, true);
}

void* MapNodeCopy(const void* other) {
    if (!MapNode_IsValid(other)) return NULL;
    
    MapNode* node = MapNode_Cast((void*)other);
    if (!node) return NULL;
    
    return node->copy();
}

/* ======================================================================== */
/* Memory Management                                                        */
/* ======================================================================== */

void MapNode_Free(void* ptr) {
    if (!MapNode_IsValid(ptr)) return;
    
    MapNode* node = MapNode_Cast(ptr);
    if (node) {
        node->free();
    }
}

/* ======================================================================== */
/* Utility Functions                                                        */
/* ======================================================================== */

int MapNode_ToString(const void* ptr, char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return -1;
    
    buffer[0] = '\0';  /* Ensure null termination */
    
    if (!MapNode_IsValid(ptr)) {
        return snprintf(buffer, buffer_size, "<invalid>");
    }
    
    MapNode* node = MapNode_Cast((void*)ptr);
    if (!node) {
        return snprintf(buffer, buffer_size, "<cast_failed>");
    }
    
    const char* type_name = node->typeName();
    size_t size = node->size();
    
    switch (node->type()) {
        case MAPNODE_TYPE_INT:
            return snprintf(buffer, buffer_size, "%s(%d)", type_name, node->asInt());
        case MAPNODE_TYPE_FLOAT:
            return snprintf(buffer, buffer_size, "%s(%.2f)", type_name, node->asFloat());
        case MAPNODE_TYPE_DOUBLE:
            return snprintf(buffer, buffer_size, "%s(%.2lf)", type_name, node->asDouble());
        case MAPNODE_TYPE_STRING:
            return snprintf(buffer, buffer_size, "%s(\"%s\")", type_name, 
                           node->asString() ? node->asString() : "NULL");
        case MAPNODE_TYPE_POINTER:
            return snprintf(buffer, buffer_size, "%s(%p)", type_name, node->asPointer());
        case MAPNODE_TYPE_BYTES:
            return snprintf(buffer, buffer_size, "%s(%zu bytes)", type_name, size);
        default:
            return snprintf(buffer, buffer_size, "%s(?)", type_name);
    }
}

int MapNode_Compare(const void* a, const void* b) {
    /* Handle NULL cases */
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    
    /* Validate both are MapNodes */
    if (!MapNode_IsValid(a) || !MapNode_IsValid(b)) {
        return (MapNode_IsValid(a) ? 1 : 0) - (MapNode_IsValid(b) ? 1 : 0);
    }
    
    MapNode* node_a = MapNode_Cast((void*)a);
    MapNode* node_b = MapNode_Cast((void*)b);
    
    if (!node_a || !node_b) return 0;
    
    /* Compare types first */
    MapNodeType type_a = node_a->type();
    MapNodeType type_b = node_b->type();
    
    if (type_a != type_b) {
        return (int)type_a - (int)type_b;
    }
    
    /* Compare values based on type */
    switch (type_a) {
        case MAPNODE_TYPE_INT: {
            int val_a = node_a->asInt();
            int val_b = node_b->asInt();
            return (val_a > val_b) - (val_a < val_b);
        }
        case MAPNODE_TYPE_FLOAT: {
            float val_a = node_a->asFloat();
            float val_b = node_b->asFloat();
            return (val_a > val_b) - (val_a < val_b);
        }
        case MAPNODE_TYPE_DOUBLE: {
            double val_a = node_a->asDouble();
            double val_b = node_b->asDouble();
            return (val_a > val_b) - (val_a < val_b);
        }
        case MAPNODE_TYPE_STRING: {
            const char* str_a = node_a->asString();
            const char* str_b = node_b->asString();
            if (!str_a && !str_b) return 0;
            if (!str_a) return -1;
            if (!str_b) return 1;
            return strcmp(str_a, str_b);
        }
        case MAPNODE_TYPE_POINTER: {
            void* ptr_a = node_a->asPointer();
            void* ptr_b = node_b->asPointer();
            return (ptr_a > ptr_b) - (ptr_a < ptr_b);
        }
        case MAPNODE_TYPE_BYTES: {
            size_t size_a, size_b;
            const void* data_a = node_a->asBytes(&size_a);
            const void* data_b = node_b->asBytes(&size_b);
            
            if (size_a != size_b) {
                return (size_a > size_b) - (size_a < size_b);
            }
            
            if (!data_a && !data_b) return 0;
            if (!data_a) return -1;
            if (!data_b) return 1;
            
            return memcmp(data_a, data_b, size_a);
        }
        default:
            return 0;
    }
}

size_t MapNode_Hash(const void* ptr) {
    if (!MapNode_IsValid(ptr)) return 0;
    
    MapNode* node = MapNode_Cast((void*)ptr);
    if (!node) return 0;
    
    /* Simple djb2-style hash incorporating type and value */
    size_t hash = 5381;
    
    /* Hash the type first */
    hash = ((hash << 5) + hash) + (size_t)node->type();
    
    /* Hash the value based on type */
    switch (node->type()) {
        case MAPNODE_TYPE_INT: {
            int val = node->asInt();
            hash = ((hash << 5) + hash) + (size_t)val;
            break;
        }
        case MAPNODE_TYPE_FLOAT: {
            /* Convert to int for hashing (simple approach) */
            union { float f; uint32_t i; } converter;
            converter.f = node->asFloat();
            hash = ((hash << 5) + hash) + converter.i;
            break;
        }
        case MAPNODE_TYPE_DOUBLE: {
            /* Convert to int for hashing (simple approach) */
            union { double d; uint64_t i; } converter;
            converter.d = node->asDouble();
            hash = ((hash << 5) + hash) + (size_t)converter.i;
            break;
        }
        case MAPNODE_TYPE_STRING: {
            const char* str = node->asString();
            if (str) {
                while (*str) {
                    hash = ((hash << 5) + hash) + (unsigned char)*str;
                    str++;
                }
            }
            break;
        }
        case MAPNODE_TYPE_POINTER: {
            uintptr_t addr = (uintptr_t)node->asPointer();
            hash = ((hash << 5) + hash) + (size_t)addr;
            break;
        }
        case MAPNODE_TYPE_BYTES: {
            size_t size;
            const unsigned char* data = (const unsigned char*)node->asBytes(&size);
            if (data) {
                size_t i;
                for (i = 0; i < size && i < 64; i++) {  /* Limit to first 64 bytes */
                    hash = ((hash << 5) + hash) + data[i];
                }
            }
            break;
        }
    }
    
    return hash;
}