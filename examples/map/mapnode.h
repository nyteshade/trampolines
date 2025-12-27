/**
 * @file mapnode.h
 * @brief MapNode - Smart pointer with type introspection for maps
 *
 * MapNode provides a zero-cognitive-load wrapper for map keys and values
 * that eliminates size management while adding runtime type safety through
 * magic byte validation. Any void* can be tested for MapNode validity and
 * cast to the appropriate interface.
 *
 * Memory layout allows void* pointers to be introspected:
 *   void* data     <- The pointer passed around (offset 0)
 *   uint32_t magic <- Magic bytes for validation (offset sizeof(void*))
 *   uint32_t type  <- Type identifier (offset sizeof(void*) + sizeof(uint32_t))
 *   ... rest of private data ...
 *   MapNode public <- Trampoline interface
 *
 * @author Trampoline Map Example
 * @date 2025
 */

#ifndef MAPNODE_H
#define MAPNODE_H

#include <trampoline/trampoline.h>
#include <trampoline/macros.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* ======================================================================== */
/* Magic Byte Constants                                                     */
/* ======================================================================== */

#define MAPNODE_MAGIC_INT     0x4D4E696E  /* 'MNin' (MapNode iNt) */
#define MAPNODE_MAGIC_FLOAT   0x4D4E666C  /* 'MNfl' (MapNode fLoat) */
#define MAPNODE_MAGIC_DOUBLE  0x4D4E6462  /* 'MNdb' (MapNode DouBle) */
#define MAPNODE_MAGIC_STRING  0x4D4E7374  /* 'MNst' (MapNode STring) */
#define MAPNODE_MAGIC_POINTER 0x4D4E7074  /* 'MNpt' (MapNode PoinTer) */
#define MAPNODE_MAGIC_BYTES   0x4D4E6279  /* 'MNby' (MapNode BYtes) */

/* ======================================================================== */
/* Type Enumeration                                                         */
/* ======================================================================== */

typedef enum MapNodeType {
    MAPNODE_TYPE_INT = 1,
    MAPNODE_TYPE_FLOAT,
    MAPNODE_TYPE_DOUBLE,
    MAPNODE_TYPE_STRING,
    MAPNODE_TYPE_POINTER,
    MAPNODE_TYPE_BYTES
} MapNodeType;

/* ======================================================================== */
/* Public MapNode Interface                                                 */
/* ======================================================================== */

/**
 * @struct MapNode
 * @brief Public interface for type-safe map elements
 *
 * MapNode provides a trampoline-based interface for accessing and
 * manipulating typed data without requiring explicit size management.
 * All methods are zero-parameter (except where noted) thanks to the
 * trampoline pattern automatically injecting the self pointer.
 *
 * @example Basic usage
 * @code
 * void* key = MapNodeFromString("username");
 * void* value = MapNodeFromInt(42);
 *
 * // Can treat as void* for APIs
 * map->put(key, value);
 *
 * // Or cast to get rich interface
 * MapNode* smart_key = MapNode_Cast(key);
 * if (smart_key) {
 *     printf("Key: %s (size: %zu)\n",
 *            smart_key->asString(), smart_key->size());
 * }
 *
 * MapNode_Free(key);
 * MapNode_Free(value);
 * @endcode
 */
typedef struct MapNode {
    /**
     * @brief Get the value as an integer
     * @return Integer value, or 0 if not an integer type
     */
    int (*asInt)();

    /**
     * @brief Get the value as a float
     * @return Float value, or 0.0f if not a float type
     */
    float (*asFloat)();

    /**
     * @brief Get the value as a double
     * @return Double value, or 0.0 if not a double type
     */
    double (*asDouble)();

    /**
     * @brief Get the value as a string
     * @return String pointer, or NULL if not a string type
     * @note Returned pointer is valid until the MapNode is freed
     */
    const char* (*asString)();

    /**
     * @brief Get the value as a generic pointer
     * @return Pointer value, or NULL if not a pointer type
     */
    void* (*asPointer)();

    /**
     * @brief Get the value as raw bytes
     * @param out_size Pointer to store the size of the data
     * @return Pointer to raw data, or NULL if error
     * @note Returned pointer is valid until the MapNode is freed
     */
    const void* (*asBytes)(size_t* out_size);

    /**
     * @brief Check if this is an integer type
     * @return true if integer, false otherwise
     */
    bool (*isInt)();

    /**
     * @brief Check if this is a float type
     * @return true if float, false otherwise
     */
    bool (*isFloat)();

    /**
     * @brief Check if this is a double type
     * @return true if double, false otherwise
     */
    bool (*isDouble)();

    /**
     * @brief Check if this is a string type
     * @return true if string, false otherwise
     */
    bool (*isString)();

    /**
     * @brief Check if this is a pointer type
     * @return true if pointer, false otherwise
     */
    bool (*isPointer)();

    /**
     * @brief Check if this is a raw bytes type
     * @return true if bytes, false otherwise
     */
    bool (*isBytes)();

    /**
     * @brief Get a human-readable type name
     * @return String describing the type (e.g., "int", "string")
     */
    const char* (*typeName)();

    /**
     * @brief Get the size of the stored data
     * @return Size in bytes of the stored data
     */
    size_t (*size)();

    /**
     * @brief Get the MapNodeType enum value
     * @return MapNodeType enum corresponding to the stored type
     */
    MapNodeType (*type)();

    /**
     * @brief Create a deep copy of this MapNode
     * @return New MapNode with copied data, or NULL on failure
     * @note Caller owns the returned MapNode and must free it
     */
    void* (*copy)();  /* Returns void* so it can be used directly with APIs */

    /**
     * @brief Free this MapNode and all associated data
     * @warning Do not use the MapNode after calling this
     */
    void (*free)();
} MapNode;

/* ======================================================================== */
/* Introspection Functions (work on any void*)                             */
/* ======================================================================== */

/**
 * @brief Check if a void* is a valid MapNode
 * @param ptr Pointer to test
 * @return true if ptr points to a valid MapNode, false otherwise
 * @note Safe to call on any pointer, including NULL
 */
bool MapNode_IsValid(const void* ptr);

/**
 * @brief Get the type of a MapNode without full casting
 * @param ptr Pointer to test (should be MapNode_IsValid first)
 * @return MapNodeType enum, or 0 if invalid
 */
MapNodeType MapNode_GetType(const void* ptr);

/**
 * @brief Get the magic bytes from a potential MapNode
 * @param ptr Pointer to test
 * @return Magic bytes value, or 0 if invalid/NULL
 * @note Primarily for debugging and validation
 */
uint32_t MapNode_GetMagic(const void* ptr);

/**
 * @brief Cast a void* to MapNode interface if valid
 * @param ptr Pointer to cast
 * @return MapNode interface pointer, or NULL if invalid
 * @note Always check return value before use
 */
MapNode* MapNode_Cast(void* ptr);

/**
 * @brief Get size of data without casting to MapNode
 * @param ptr Pointer to MapNode
 * @return Size of stored data, or 0 if invalid
 */
size_t MapNode_GetSize(const void* ptr);

/* ======================================================================== */
/* Constructor Functions                                                    */
/* ======================================================================== */

/**
 * @brief Create a MapNode containing an integer
 * @param value Integer value to store
 * @return MapNode as void*, or NULL on failure
 */
void* MapNodeFromInt(int value);

/**
 * @brief Create a MapNode containing a float
 * @param value Float value to store
 * @return MapNode as void*, or NULL on failure
 */
void* MapNodeFromFloat(float value);

/**
 * @brief Create a MapNode containing a double
 * @param value Double value to store
 * @return MapNode as void*, or NULL on failure
 */
void* MapNodeFromDouble(double value);

/**
 * @brief Create a MapNode containing a string
 * @param str String to store (will be copied)
 * @return MapNode as void*, or NULL on failure
 * @note The string is copied, so the original can be freed
 */
void* MapNodeFromString(const char* str);

/**
 * @brief Create a MapNode containing a pointer
 * @param ptr Pointer value to store
 * @return MapNode as void*, or NULL on failure
 * @note Only stores the pointer value, doesn't manage the pointed-to data
 */
void* MapNodeFromPointer(void* ptr);

/**
 * @brief Create a MapNode containing raw bytes
 * @param data Data to store (will be copied)
 * @param size Size of data in bytes
 * @return MapNode as void*, or NULL on failure
 * @note The data is copied, so the original can be freed
 */
void* MapNodeFromBytes(const void* data, size_t size);

/**
 * @brief Create a MapNode by copying another MapNode
 * @param other MapNode to copy (can be void* or MapNode*)
 * @return New MapNode as void*, or NULL on failure
 */
void* MapNodeCopy(const void* other);

/* ======================================================================== */
/* Memory Management                                                        */
/* ======================================================================== */

/**
 * @brief Free a MapNode (works on void*)
 * @param ptr MapNode to free (void* or MapNode*)
 * @note Safe to call on NULL, validates pointer before freeing
 */
void MapNode_Free(void* ptr);

/* ======================================================================== */
/* Utility Functions                                                        */
/* ======================================================================== */

/**
 * @brief Get a human-readable string representation
 * @param ptr MapNode to represent (void* or MapNode*)
 * @param buffer Buffer to write string to
 * @param buffer_size Size of buffer
 * @return Number of characters written (excluding null terminator)
 * @note Buffer will always be null-terminated if buffer_size > 0
 */
int MapNode_ToString(const void* ptr, char* buffer, size_t buffer_size);

/**
 * @brief Compare two MapNodes for equality
 * @param a First MapNode
 * @param b Second MapNode
 * @return 0 if equal, non-zero if different
 * @note Compares both type and value
 */
int MapNode_Compare(const void* a, const void* b);

/**
 * @brief Hash a MapNode for use in hash tables
 * @param ptr MapNode to hash
 * @return Hash value
 * @note Consistent hash based on type and value
 */
size_t MapNode_Hash(const void* ptr);

#endif /* MAPNODE_H */
