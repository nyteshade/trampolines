/**
 * @file map_v2.h
 * @brief Map (hash table) with MapNode integration for zero-cognitive-load usage
 *
 * This is the next generation Map API that integrates with MapNode to eliminate
 * the need for size management while adding powerful type introspection.
 * All void* parameters are expected to be MapNodes, but the API gracefully
 * handles validation and provides helpful error information.
 *
 * @author Trampoline Map Example
 * @date 2025
 */

#ifndef MAP_V2_H
#define MAP_V2_H

#include "../../trampoline.h"
#include <stddef.h>
#include <stdbool.h>
#include "mapnode.h"

/**
 * @struct Map
 * @brief Hash table with zero-cognitive-load MapNode interface
 *
 * This Map uses MapNode for both keys and values, eliminating size management
 * and providing automatic type safety and introspection. The API accepts
 * void* pointers which are validated as MapNodes internally.
 *
 * Key Features:
 * - Zero size management - MapNode handles all size tracking
 * - Type safety - Runtime validation of all parameters
 * - Introspection - Can examine any key or value for debugging
 * - Flexible hashing - Uses MapNode_Hash by default, but customizable
 * - Memory safe - All MapNodes are properly managed
 *
 * @example Basic usage with automatic type handling
 * @code
 * Map* map = MapMake();
 * 
 * void* key = MapNodeFromString("username");
 * void* value = MapNodeFromString("john_doe");
 * map->put(key, value);
 * 
 * void* result = map->get(key);
 * if (result) {
 *     MapNode* node = MapNode_Cast(result);
 *     printf("Username: %s\n", node->asString());
 * }
 * 
 * // Convenience methods eliminate casting
 * const char* username = map->getString(key);
 * printf("Username: %s\n", username ? username : "not found");
 * 
 * map->free();
 * @endcode
 *
 * @example Mixed type usage
 * @code
 * Map* map = MapMake();
 * 
 * void* user_id = MapNodeFromInt(123);
 * void* user_name = MapNodeFromString("Alice");
 * void* user_age = MapNodeFromInt(30);
 * void* user_score = MapNodeFromFloat(95.5);
 * 
 * map->put(user_id, user_name);
 * map->put(user_name, user_age);
 * map->put(user_age, user_score);
 * 
 * // Type-safe retrieval
 * int age = map->getInt(user_name, -1);
 * float score = map->getFloat(user_age, 0.0f);
 * 
 * printf("Age: %d, Score: %.1f\n", age, score);
 * 
 * map->free();
 * @endcode
 */
typedef struct Map {
    /* ================================================================ */
    /* Core Map Operations (accept any void*, validate as MapNode)     */
    /* ================================================================ */
    
    /**
     * @brief Insert or update a key-value pair
     * @param key MapNode key (as void*)
     * @param value MapNode value (as void*)
     * @return true if successful, false if key or value invalid
     * @note Both key and value must be valid MapNodes
     */
    bool (*put)(void* key, void* value);
    
    /**
     * @brief Retrieve a value by key
     * @param key MapNode key (as void*)
     * @return MapNode value (as void*), or NULL if not found or key invalid
     * @note Returned pointer is valid until the key is removed or map freed
     */
    void* (*get)(void* key);
    
    /**
     * @brief Remove a key-value pair from the map
     * @param key MapNode key (as void*)
     * @return true if key was found and removed, false if not found or invalid
     */
    bool (*remove)(void* key);
    
    /**
     * @brief Check if a key exists in the map
     * @param key MapNode key (as void*)
     * @return true if key exists, false if not found or key invalid
     */
    bool (*contains)(void* key);
    
    /* ================================================================ */
    /* Type-Safe Convenience Methods                                    */
    /* ================================================================ */
    
    /**
     * @brief Insert an integer value with given key
     * @param key MapNode key (as void*)
     * @param value Integer value to store
     * @return true if successful, false if key invalid
     */
    bool (*putInt)(void* key, int value);
    
    /**
     * @brief Insert a float value with given key
     * @param key MapNode key (as void*)
     * @param value Float value to store
     * @return true if successful, false if key invalid
     */
    bool (*putFloat)(void* key, float value);
    
    /**
     * @brief Insert a double value with given key
     * @param key MapNode key (as void*)
     * @param value Double value to store
     * @return true if successful, false if key invalid
     */
    bool (*putDouble)(void* key, double value);
    
    /**
     * @brief Insert a string value with given key
     * @param key MapNode key (as void*)
     * @param value String value to store (will be copied)
     * @return true if successful, false if key invalid or string NULL
     */
    bool (*putString)(void* key, const char* value);
    
    /**
     * @brief Insert a pointer value with given key
     * @param key MapNode key (as void*)
     * @param value Pointer value to store
     * @return true if successful, false if key invalid
     */
    bool (*putPointer)(void* key, void* value);
    
    /**
     * @brief Retrieve an integer value
     * @param key MapNode key (as void*)
     * @param default_value Value to return if not found or wrong type
     * @return Integer value, or default_value if not found/invalid
     */
    int (*getInt)(void* key, int default_value);
    
    /**
     * @brief Retrieve a float value
     * @param key MapNode key (as void*)
     * @param default_value Value to return if not found or wrong type
     * @return Float value, or default_value if not found/invalid
     */
    float (*getFloat)(void* key, float default_value);
    
    /**
     * @brief Retrieve a double value
     * @param key MapNode key (as void*)
     * @param default_value Value to return if not found or wrong type
     * @return Double value, or default_value if not found/invalid
     */
    double (*getDouble)(void* key, double default_value);
    
    /**
     * @brief Retrieve a string value
     * @param key MapNode key (as void*)
     * @return String pointer, or NULL if not found/invalid/wrong type
     * @note Returned pointer is valid until the key is removed or map freed
     */
    const char* (*getString)(void* key);
    
    /**
     * @brief Retrieve a pointer value
     * @param key MapNode key (as void*)
     * @return Pointer value, or NULL if not found/invalid/wrong type
     */
    void* (*getPointer)(void* key);
    
    /* ================================================================ */
    /* Map Information and Management                                   */
    /* ================================================================ */
    
    /**
     * @brief Get the number of key-value pairs in the map
     * @return Current number of entries
     */
    size_t (*size)();
    
    /**
     * @brief Check if the map is empty
     * @return true if map has no entries, false otherwise
     */
    bool (*isEmpty)();
    
    /**
     * @brief Get the current capacity (bucket count) of the map
     * @return Number of buckets in the hash table
     */
    size_t (*capacity)();
    
    /**
     * @brief Get the current load factor (size/capacity)
     * @return Load factor as a float between 0.0 and 1.0+
     */
    float (*loadFactor)();
    
    /**
     * @brief Remove all entries from the map
     * @note Frees all MapNodes stored as keys and values
     */
    void (*clear)();
    
    /**
     * @brief Force a rebuild of the hash table with new capacity
     * @param new_capacity Desired number of buckets (will be rounded to power of 2)
     * @note Expensive operation - use sparingly
     */
    void (*resize)(size_t new_capacity);
    
    /* ================================================================ */
    /* Bulk Operations                                                  */
    /* ================================================================ */
    
    /**
     * @brief Get all keys in the map
     * @param out_count Pointer to store the number of keys returned
     * @return Array of MapNode keys (as void*), caller must free the array
     * @note MapNodes remain owned by the map, only free the returned array
     */
    void** (*getAllKeys)(size_t* out_count);
    
    /**
     * @brief Get all values in the map
     * @param out_count Pointer to store the number of values returned
     * @return Array of MapNode values (as void*), caller must free the array
     * @note MapNodes remain owned by the map, only free the returned array
     */
    void** (*getAllValues)(size_t* out_count);
    
    /* ================================================================ */
    /* Debugging and Introspection                                     */
    /* ================================================================ */
    
    /**
     * @brief Print a human-readable representation of the map
     * @param max_entries Maximum number of entries to print (0 = all)
     * @note Useful for debugging, prints to stdout
     */
    void (*debug)(size_t max_entries);
    
    /**
     * @brief Validate all MapNodes in the map
     * @return Number of invalid MapNodes found (0 = all valid)
     * @note Useful for detecting corruption, prints issues to stderr
     */
    size_t (*validate)();
    
    /**
     * @brief Get statistics about the map
     * @param stats Pointer to structure to fill with statistics
     * @return true if successful, false if stats pointer NULL
     */
    bool (*getStats)(void* stats);  /* Use void* to avoid forward declaration issues */
    
    /* ================================================================ */
    /* Memory Management                                                */
    /* ================================================================ */
    
    /**
     * @brief Free the Map and all contained MapNodes
     * @warning Do not use the map after calling this
     * @note This will free all MapNode keys and values
     */
    void (*free)();
} Map;

/* ======================================================================== */
/* Map Statistics Structure                                                 */
/* ======================================================================== */

/**
 * @struct MapStats
 * @brief Statistics about a Map instance
 */
struct MapStats {
    size_t entry_count;          /**< Number of key-value pairs */
    size_t bucket_count;         /**< Number of hash buckets */
    size_t empty_buckets;        /**< Number of unused buckets */
    size_t max_chain_length;     /**< Longest collision chain */
    float load_factor;           /**< Current load factor */
    float average_chain_length;  /**< Average collision chain length */
    size_t total_memory;         /**< Approximate memory usage in bytes */
    
    /* Type distribution */
    size_t int_keys, int_values;
    size_t float_keys, float_values;
    size_t double_keys, double_values;
    size_t string_keys, string_values;
    size_t pointer_keys, pointer_values;
    size_t bytes_keys, bytes_values;
};

/* ======================================================================== */
/* Map Creation Functions                                                   */
/* ======================================================================== */

/**
 * @brief Create a new Map with default settings
 * @return New Map instance or NULL on failure
 * @note Uses MapNode_Hash and MapNode_Compare internally
 * @note Initial capacity: 16, max load factor: 0.75
 */
Map* MapMake(void);

/**
 * @brief Create a new Map with custom initial capacity
 * @param initial_capacity Starting number of buckets (rounded to power of 2)
 * @return New Map instance or NULL on failure
 */
Map* MapMakeWithCapacity(size_t initial_capacity);

/* ======================================================================== */
/* Utility Functions                                                        */
/* ======================================================================== */

/**
 * @brief Create a MapNode key-value pair for bulk operations
 * @param key MapNode key
 * @param value MapNode value
 * @return Pair structure, or {NULL, NULL} if either parameter invalid
 */
struct MapPair {
    void* key;
    void* value;
};

struct MapPair MapPair_Make(void* key, void* value);

/**
 * @brief Insert multiple key-value pairs at once
 * @param map Map to insert into
 * @param pairs Array of MapPair structures
 * @param count Number of pairs to insert
 * @return Number of pairs successfully inserted
 */
size_t Map_PutAll(Map* map, struct MapPair* pairs, size_t count);

#endif /* MAP_V2_H */