/* ======================================================================== */
/* JSON Trampoline Class                                                   */
/* ======================================================================== */

#ifndef TRAMPOLINE_JSON_H
#define TRAMPOLINE_JSON_H

#include <stddef.h>

/* C89-compatible boolean type */
#ifndef TRAMPOLINE_BOOL_DEFINED
#define TRAMPOLINE_BOOL_DEFINED
  #ifndef __cplusplus
    #ifdef __STDC_VERSION__
      #if __STDC_VERSION__ >= 199901L
        #include <stdbool.h>
      #else
        /* C89 mode */
        typedef int bool;
        #define true 1
        #define false 0
      #endif
    #else
      /* C89 mode (no __STDC_VERSION__) */
      typedef int bool;
      #define true 1
      #define false 0
    #endif
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct Json Json;
typedef struct JsonArray JsonArray;
typedef struct JsonObject JsonObject;

/* JSON Value Types */
typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

/* ======================================================================== */
/* JSON Value Class                                                        */
/* ======================================================================== */

struct Json {
    /* Type inspection */
    JsonType (*type)(void);
    bool (*isNull)(void);
    bool (*isBool)(void);
    bool (*isNumber)(void);
    bool (*isString)(void);
    bool (*isArray)(void);
    bool (*isObject)(void);
    
    /* Value getters */
    bool (*getBool)(void);
    double (*getNumber)(void);
    const char* (*getString)(void);
    JsonArray* (*getArray)(void);
    JsonObject* (*getObject)(void);
    
    /* Value setters */
    void (*setNull)(void);
    void (*setBool)(bool value);
    void (*setNumber)(double value);
    void (*setString)(const char* value);
    void (*setArray)(void);
    void (*setObject)(void);
    
    /* Array operations (when type is array) */
    size_t (*arraySize)(void);
    Json* (*arrayGet)(size_t index);
    void (*arrayAdd)(Json* value);
    void (*arrayInsert)(size_t index, Json* value);
    void (*arrayRemove)(size_t index);
    void (*arrayClear)(void);
    
    /* Object operations (when type is object) */
    size_t (*objectSize)(void);
    bool (*objectHas)(const char* key);
    Json* (*objectGet)(const char* key);
    void (*objectSet)(const char* key, Json* value);
    void (*objectRemove)(const char* key);
    void (*objectClear)(void);
    const char** (*objectKeys)(size_t* count);
    
    /* Serialization */
    char* (*stringify)(void);
    char* (*prettyPrint)(int indent_size);
    
    /* Utility */
    Json* (*clone)(void);
    bool (*equals)(Json* other);
    void (*free)(void);
};

/* ======================================================================== */
/* JSON Array Class (convenience wrapper)                                  */
/* ======================================================================== */

struct JsonArray {
    /* Core operations */
    size_t (*size)(void);
    Json* (*get)(size_t index);
    void (*add)(Json* value);
    void (*insert)(size_t index, Json* value);
    void (*remove)(size_t index);
    void (*clear)(void);
    
    /* Convenience methods */
    void (*addNull)(void);
    void (*addBool)(bool value);
    void (*addNumber)(double value);
    void (*addString)(const char* value);
    Json* (*addArray)(void);
    Json* (*addObject)(void);
    
    /* Type-specific getters */
    bool (*getBool)(size_t index);
    double (*getNumber)(size_t index);
    const char* (*getString)(size_t index);
    
    /* Iteration */
    void (*forEach)(void (*callback)(size_t index, Json* value, void* context), void* context);
    
    /* Conversion */
    Json* (*toJson)(void);
    void (*free)(void);
};

/* ======================================================================== */
/* JSON Object Class (convenience wrapper)                                 */
/* ======================================================================== */

struct JsonObject {
    /* Core operations */
    size_t (*size)(void);
    bool (*has)(const char* key);
    Json* (*get)(const char* key);
    void (*set)(const char* key, Json* value);
    void (*remove)(const char* key);
    void (*clear)(void);
    const char** (*keys)(size_t* count);
    
    /* Convenience methods */
    void (*setNull)(const char* key);
    void (*setBool)(const char* key, bool value);
    void (*setNumber)(const char* key, double value);
    void (*setString)(const char* key, const char* value);
    Json* (*setArray)(const char* key);
    Json* (*setObject)(const char* key);
    
    /* Type-specific getters */
    bool (*getBool)(const char* key);
    double (*getNumber)(const char* key);
    const char* (*getString)(const char* key);
    
    /* Iteration */
    void (*forEach)(void (*callback)(const char* key, Json* value, void* context), void* context);
    
    /* Conversion */
    Json* (*toJson)(void);
    void (*free)(void);
};

/* ======================================================================== */
/* Factory Functions                                                       */
/* ======================================================================== */

/* Create JSON values */
Json* JsonMakeNull(void);
Json* JsonMakeBool(bool value);
Json* JsonMakeNumber(double value);
Json* JsonMakeString(const char* value);
Json* JsonMakeArray(void);
Json* JsonMakeObject(void);

/* Parse JSON */
Json* JsonParse(const char* json_string);
Json* JsonParseFile(const char* filename);

/* Create convenience wrappers */
JsonArray* JsonArrayMake(void);
JsonObject* JsonObjectMake(void);

#ifdef __cplusplus
}
#endif

#endif /* TRAMPOLINE_JSON_H */
