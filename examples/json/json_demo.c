/* ======================================================================== */
/* JSON Trampoline Example                                                 */
/* ======================================================================== */

#include <trampoline/macros.h>
#include <trampoline/classes/json.h>
#include <trampoline/classes/string.h>
#include <stdio.h>
#include <stdlib.h>

void demonstrate_basic_types(void) {
    Json* obj;
    Json* name;
    Json* age;
    Json* active;
    char* json_str;

    printf("\n=== Basic JSON Types ===\n");

    /* Create a simple object */
    obj = JsonMakeObject();

    /* Add different types */
    name = JsonMakeString("John Doe");
    obj->objectSet("name", name);
    name->free();

    age = JsonMakeNumber(30);
    obj->objectSet("age", age);
    age->free();

    active = JsonMakeBool(true);
    obj->objectSet("active", active);
    active->free();

    /* Print the JSON */
    json_str = obj->stringify();
    printf("JSON: %s\n", json_str);
    free(json_str);

    /* Pretty print */
    json_str = obj->prettyPrint(2);
    printf("\nPretty JSON:\n%s\n", json_str);
    free(json_str);

    obj->free();
}

void demonstrate_arrays(void) {
    Json* arr;
    Json* item;
    Json* nested;
    char* json_str;
    size_t i;

    printf("\n=== JSON Arrays ===\n");

    /* Create an array */
    arr = JsonMakeArray();

    /* Add various items */
    item = JsonMakeString("first");
    arr->arrayAdd(item);
    item->free();

    item = JsonMakeNumber(42);
    arr->arrayAdd(item);
    item->free();

    item = JsonMakeBool(false);
    arr->arrayAdd(item);
    item->free();

    /* Add a nested array */
    nested = JsonMakeArray();
    item = JsonMakeNumber(1);
    nested->arrayAdd(item);
    item->free();

    item = JsonMakeNumber(2);
    nested->arrayAdd(item);
    item->free();

    item = JsonMakeNumber(3);
    nested->arrayAdd(item);
    item->free();

    arr->arrayAdd(nested);
    nested->free();

    /* Print array */
    json_str = arr->stringify();
    printf("Array: %s\n", json_str);
    free(json_str);

    /* Access array elements */
    printf("\nArray has %zu elements:\n", arr->arraySize());
    for (i = 0; i < arr->arraySize(); i++) {
        Json* elem = arr->arrayGet(i);
        if (elem) {
            json_str = elem->stringify();
            printf("  [%zu]: %s (type: %d)\n", i, json_str, elem->type());
            free(json_str);
            elem->free();
        }
    }

    arr->free();
}

void demonstrate_nested_objects(void) {
    Json* root;
    Json* user;
    Json* address;
    Json* coords;
    Json* tags;
    Json* tag;
    char* json_str;

    printf("\n=== Nested JSON Objects ===\n");

    /* Create root object */
    root = JsonMakeObject();

    /* Create user object */
    user = JsonMakeObject();
    tag = JsonMakeString("alice123");
    user->objectSet("username", tag);
    tag->free();

    tag = JsonMakeString("alice@example.com");
    user->objectSet("email", tag);
    tag->free();

    /* Create address object */
    address = JsonMakeObject();
    tag = JsonMakeString("123 Main St");
    address->objectSet("street", tag);
    tag->free();

    tag = JsonMakeString("Springfield");
    address->objectSet("city", tag);
    tag->free();

    tag = JsonMakeString("12345");
    address->objectSet("zip", tag);
    tag->free();

    /* Create coordinates */
    coords = JsonMakeObject();
    tag = JsonMakeNumber(40.7128);
    coords->objectSet("lat", tag);
    tag->free();

    tag = JsonMakeNumber(-74.0060);
    coords->objectSet("lng", tag);
    tag->free();

    address->objectSet("coordinates", coords);
    coords->free();

    user->objectSet("address", address);
    address->free();

    /* Create tags array */
    tags = JsonMakeArray();

    tag = JsonMakeString("developer");
    tags->arrayAdd(tag);
    tag->free();

    tag = JsonMakeString("javascript");
    tags->arrayAdd(tag);
    tag->free();

    tag = JsonMakeString("python");
    tags->arrayAdd(tag);
    tag->free();

    user->objectSet("tags", tags);
    tags->free();

    /* Add user to root */
    root->objectSet("user", user);
    user->free();

    /* Add timestamp */
    tag = JsonMakeNumber(1234567890);
    root->objectSet("timestamp", tag);
    tag->free();

    /* Pretty print the entire structure */
    json_str = root->prettyPrint(2);
    printf("Nested structure:\n%s\n", json_str);
    free(json_str);

    root->free();
}

void demonstrate_parsing(void) {
    const char* json_string;
    Json* parsed;
    Json* item;
    char* json_str;

    printf("\n=== JSON Parsing ===\n");

    /* Parse a simple JSON string */
    json_string = "{\"message\":\"Hello, World!\",\"count\":42,\"valid\":true}";
    printf("Parsing: %s\n", json_string);

    parsed = JsonParse(json_string);
    if (parsed) {
        /* Access parsed values */
        if (parsed->isObject()) {
            printf("Successfully parsed an object with %zu keys\n", parsed->objectSize());

            /* Get message */
            item = parsed->objectGet("message");
            if (item && item->isString()) {
                printf("Message: %s\n", item->getString());
            }
            if (item) item->free();

            /* Get count */
            item = parsed->objectGet("count");
            if (item && item->isNumber()) {
                printf("Count: %.0f\n", item->getNumber());
            }
            if (item) item->free();

            /* Get valid */
            item = parsed->objectGet("valid");
            if (item && item->isBool()) {
                printf("Valid: %s\n", item->getBool() ? "true" : "false");
            }
            if (item) item->free();
        }

        parsed->free();
    } else {
        printf("Failed to parse JSON\n");
    }

    /* Parse an array */
    json_string = "[1, 2, \"three\", {\"four\": 4}, [5, 6]]";
    printf("\nParsing array: %s\n", json_string);

    parsed = JsonParse(json_string);
    if (parsed && parsed->isArray()) {
        size_t i;
        printf("Array with %zu elements:\n", parsed->arraySize());

        for (i = 0; i < parsed->arraySize(); i++) {
            item = parsed->arrayGet(i);
            if (item) {
                json_str = item->stringify();
                printf("  [%zu]: %s\n", i, json_str);
                free(json_str);
                item->free();
            }
        }

        parsed->free();
    }
}

void demonstrate_modification(void) {
    Json* obj;
    Json* arr;
    Json* item;
    char* before;
    char* after;

    printf("\n=== JSON Modification ===\n");

    /* Create initial object */
    obj = JsonMakeObject();

    item = JsonMakeString("Initial");
    obj->objectSet("status", item);
    item->free();

    item = JsonMakeNumber(1);
    obj->objectSet("version", item);
    item->free();

    before = obj->stringify();
    printf("Before: %s\n", before);
    free(before);

    /* Modify existing values */
    item = JsonMakeString("Modified");
    obj->objectSet("status", item);
    item->free();

    item = JsonMakeNumber(2);
    obj->objectSet("version", item);
    item->free();

    /* Add new values */
    arr = JsonMakeArray();
    item = JsonMakeString("change1");
    arr->arrayAdd(item);
    item->free();

    item = JsonMakeString("change2");
    arr->arrayAdd(item);
    item->free();

    obj->objectSet("changes", arr);
    arr->free();

    after = obj->stringify();
    printf("After:  %s\n", after);
    free(after);

    obj->free();
}

void demonstrate_equality(void) {
    Json* obj1;
    Json* obj2;
    Json* obj3;
    Json* item;

    printf("\n=== JSON Equality ===\n");

    /* Create first object */
    obj1 = JsonMakeObject();
    item = JsonMakeString("test");
    obj1->objectSet("name", item);
    item->free();

    item = JsonMakeNumber(123);
    obj1->objectSet("value", item);
    item->free();

    /* Clone the object */
    obj2 = obj1->clone();

    /* Create different object */
    obj3 = JsonMakeObject();
    item = JsonMakeString("test");
    obj3->objectSet("name", item);
    item->free();

    item = JsonMakeNumber(456); /* Different value */
    obj3->objectSet("value", item);
    item->free();

    /* Test equality */
    printf("obj1 equals obj2 (clone): %s\n", obj1->equals(obj2) ? "true" : "false");
    printf("obj1 equals obj3 (different): %s\n", obj1->equals(obj3) ? "true" : "false");

    obj1->free();
    obj2->free();
    obj3->free();
}

void demonstrate_with_string_class(void) {
    Json* config;
    Json* item;
    String* json_str;
    String* key;
    String* value;

    printf("\n=== JSON with String Class ===\n");

    /* Create configuration object using String class */
    config = JsonMakeObject();

    /* Use String class to build values */
    key = StringMake("application");
    value = StringMake("TramplineJSON");
    value->append(" v1.0");

    item = JsonMakeString(value->cStr());
    config->objectSet(key->cStr(), item);
    item->free();

    key->set("environment");
    value->set("production");
    item = JsonMakeString(value->cStr());
    config->objectSet(key->cStr(), item);
    item->free();

    /* Create complex key with String manipulation */
    key->set("debug");
    key->append("_");
    key->append("mode");

    item = JsonMakeBool(false);
    config->objectSet(key->cStr(), item);
    item->free();

    /* Convert to JSON string and manipulate */
    json_str = StringMake(config->stringify());

    printf("Configuration JSON:\n");

    /* Use String class features */
    if (json_str->contains("production")) {
        printf("  Environment: Production detected\n");
    }

    printf("  JSON length: %zu characters\n", json_str->length());
    printf("  Content: %s\n", json_str->cStr());

    /* Cleanup */
    key->free();
    value->free();
    json_str->free();
    config->free();
}

int main(void) {
    printf("=== JSON Trampoline Class Demo ===\n");

    demonstrate_basic_types();
    demonstrate_arrays();
    demonstrate_nested_objects();
    demonstrate_parsing();
    demonstrate_modification();
    demonstrate_equality();
    demonstrate_with_string_class();

    printf("\n=== Demo Complete ===\n");
    return 0;
}
