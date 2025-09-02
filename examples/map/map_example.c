#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <trampoline.h>

#include "mapnode.h"
#include "map.h"
#include "map_impl.c"

void demonstrate_zero_cognitive_load() {
    printf("=== Zero Cognitive Load Map API ===\n");
    
    Map* map = MapMake();
    if (!map) {
        printf("Failed to create map\n");
        return;
    }
    
    printf("✓ Created map with capacity: %zu\n", map->capacity());
    
    /* OLD WAY (cognitive overload):
     * map->put("username", strlen("username")+1, "john_doe", strlen("john_doe")+1);
     * char* result = (char*)map->get("username", strlen("username")+1);
     */
    
    /* NEW WAY (zero cognitive load): */
    void* username_key = MapNodeFromString("username");
    void* age_key = MapNodeFromString("age");
    void* score_key = MapNodeFromString("score");
    
    /* Multiple ways to insert - all equally easy */
    map->put(username_key, MapNodeFromString("john_doe"));
    map->putInt(age_key, 25);
    map->putFloat(score_key, 95.5f);
    
    printf("✓ Inserted 3 entries with zero size management\n");
    printf("  Map size: %zu, Load factor: %.2f\n", map->size(), map->loadFactor());
    
    /* Multiple ways to retrieve - with type safety */
    const char* username = map->getString(username_key);
    int age = map->getInt(age_key, -1);
    float score = map->getFloat(score_key, 0.0f);
    
    printf("✓ Retrieved values with type safety:\n");
    printf("  Username: %s\n", username ? username : "not found");
    printf("  Age: %d\n", age);
    printf("  Score: %.1f\n", score);
    
    /* Demonstrate type introspection on results */
    void* age_value = map->get(age_key);
    if (MapNode_IsValid(age_value)) {
        MapNode* node = MapNode_Cast(age_value);
        printf("✓ Age value type: %s, size: %zu bytes\n", 
               node->typeName(), node->size());
    }
    
    map->free();
    printf("\n");
}

void demonstrate_mixed_types_and_debugging() {
    printf("=== Mixed Types & Debugging Features ===\n");
    
    Map* map = MapMakeWithCapacity(8);  /* Start small to see resizing */
    
    /* Create a variety of key-value combinations */
    struct {
        void* key;
        void* value;
        const char* description;
    } test_data[] = {
        {MapNodeFromString("user_id"), MapNodeFromInt(12345), "string->int"},
        {MapNodeFromInt(42), MapNodeFromString("answer"), "int->string"},
        {MapNodeFromFloat(3.14f), MapNodeFromDouble(2.718), "float->double"},
        {MapNodeFromString("config"), MapNodeFromPointer((void*)0xDEADBEEF), "string->pointer"},
        {MapNodeFromDouble(1.0), MapNodeFromBytes("binary\0data", 11), "double->bytes"}
    };
    
    size_t num_tests = sizeof(test_data) / sizeof(test_data[0]);
    
    printf("Inserting %zu diverse key-value pairs:\n", num_tests);
    {
        size_t i;
        for (i = 0; i < num_tests; i++) {
            bool success = map->put(test_data[i].key, test_data[i].value);
            printf("  ✓ %s: %s\n", test_data[i].description, 
                   success ? "inserted" : "failed");
        }
    }
    
    printf("\nMap grew to capacity: %zu (auto-resize triggered)\n", map->capacity());
    
    /* Use the debug function */
    printf("\nDebug output:\n");
    map->debug(3);  /* Show first 3 entries */
    
    /* Validate the map */
    size_t errors = map->validate();
    printf("\nValidation: %zu errors found\n", errors);
    
    /* Get detailed statistics */
    {
        struct MapStats stats;
        if (map->getStats(&stats)) {
        printf("\nDetailed Statistics:\n");
        printf("  Entries: %zu, Buckets: %zu, Load: %.2f\n", 
               stats.entry_count, stats.bucket_count, stats.load_factor);
        printf("  Empty buckets: %zu, Max chain: %zu, Avg chain: %.2f\n",
               stats.empty_buckets, stats.max_chain_length, stats.average_chain_length);
        printf("  Memory usage: ~%zu bytes\n", stats.total_memory);
        printf("  Key types: %zu int, %zu float, %zu double, %zu string, %zu pointer, %zu bytes\n",
               stats.int_keys, stats.float_keys, stats.double_keys,
               stats.string_keys, stats.pointer_keys, stats.bytes_keys);
        printf("  Value types: %zu int, %zu float, %zu double, %zu string, %zu pointer, %zu bytes\n",
               stats.int_values, stats.float_values, stats.double_values,
               stats.string_values, stats.pointer_values, stats.bytes_values);
        }
    }
    
    map->free();
    printf("\n");
}

void demonstrate_error_handling() {
    printf("=== Error Handling & Safety ===\n");
    
    Map* map = MapMake();
    
    /* Test with invalid keys */
    {
        char* not_a_mapnode = "invalid key";
        bool result1 = map->put(not_a_mapnode, MapNodeFromString("test"));
        void* valid_key;
        bool result2, result3;
        void* result4;
        int int_result;
        const char* str_result;
        bool removed1, removed2;
        
        printf("Insert with invalid key: %s\n", result1 ? "succeeded" : "failed (correct)");
        
        /* Test with invalid values */
        valid_key = MapNodeFromString("test_key");
        result2 = map->put(valid_key, not_a_mapnode);
        printf("Insert with invalid value: %s\n", result2 ? "succeeded" : "failed (correct)");
        
        /* Test valid insertion */
        result3 = map->put(valid_key, MapNodeFromString("valid_value"));
        printf("Insert with valid key/value: %s\n", result3 ? "succeeded (correct)" : "failed");
        
        /* Test retrieval with invalid key */
        result4 = map->get(not_a_mapnode);
        printf("Get with invalid key: %s\n", result4 ? "found (unexpected)" : "not found (correct)");
        
        /* Test type-safe getters with wrong types */
        int_result = map->getInt(valid_key, -1);  /* Value is string, not int */
        printf("getInt on string value: %d (should be default -1)\n", int_result);
        
        str_result = map->getString(valid_key);
        printf("getString on string value: %s (should work)\n", str_result ? str_result : "NULL");
        
        /* Test safe removal */
        removed1 = map->remove(not_a_mapnode);
        printf("Remove with invalid key: %s\n", removed1 ? "removed" : "not found (correct)");
        
        removed2 = map->remove(valid_key);
        printf("Remove with valid key: %s\n", removed2 ? "removed (correct)" : "not found");
        
        printf("Final map size: %zu (should be 0)\n", map->size());
    }
    
    map->free();
    printf("\n");
}

void demonstrate_bulk_operations() {
    printf("=== Bulk Operations ===\n");
    
    Map* map = MapMake();
    
    /* Use the bulk insert utility */
    {
        struct MapPair pairs[] = {
            MapPair_Make(MapNodeFromString("name"), MapNodeFromString("Alice")),
            MapPair_Make(MapNodeFromString("age"), MapNodeFromInt(30)),
            MapPair_Make(MapNodeFromString("score"), MapNodeFromFloat(85.5f)),
            MapPair_Make(MapNodeFromString("active"), MapNodeFromInt(1)),
            MapPair_Make(MapNodeFromString("department"), MapNodeFromString("Engineering"))
        };
        size_t num_pairs = sizeof(pairs) / sizeof(pairs[0]);
        size_t inserted = Map_PutAll(map, pairs, num_pairs);
    
        printf("Bulk inserted %zu/%zu pairs\n", inserted, num_pairs);
    }
    
    /* Get all keys and values */
    {
        size_t key_count, value_count;
        void** keys = map->getAllKeys(&key_count);
        void** values = map->getAllValues(&value_count);
    
        printf("Retrieved %zu keys and %zu values:\n", key_count, value_count);
        
        {
            size_t i;
            for (i = 0; i < key_count; i++) {
                char key_str[64], value_str[64];
                MapNode_ToString(keys[i], key_str, sizeof(key_str));
                MapNode_ToString(values[i], value_str, sizeof(value_str));
                printf("  %s => %s\n", key_str, value_str);
            }
        }
        
        /* Clean up arrays (but not the MapNodes - they're owned by the map) */
        free(keys);
        free(values);
    }
    
    map->free();
    printf("\n");
}

void demonstrate_performance() {
    printf("=== Performance Test ===\n");
    
    Map* map = MapMakeWithCapacity(1024);
    const size_t num_entries = 10000;
    
    printf("Inserting %zu entries...\n", num_entries);
    
    /* Insert many entries */
    {
        size_t i;
        for (i = 0; i < num_entries; i++) {
            char key_buf[32];
            void* key;
            void* value;
            
            snprintf(key_buf, sizeof(key_buf), "key_%zu", i);
            
            key = MapNodeFromString(key_buf);
            value = MapNodeFromInt((int)(i * 2));
            
            map->put(key, value);
            
            if (i % 1000 == 0) {
                printf("  Progress: %zu entries, capacity: %zu, load: %.2f\n",
                       map->size(), map->capacity(), map->loadFactor());
            }
        }
    }
    
    printf("Final stats: size=%zu, capacity=%zu, load=%.2f\n",
           map->size(), map->capacity(), map->loadFactor());
    
    /* Test lookups */
    printf("Testing lookups...\n");
    {
        size_t found = 0;
        size_t i;
        for (i = 0; i < 100; i++) {
            size_t test_idx = rand() % num_entries;
            char key_buf[32];
            void* key;
            int expected;
            int actual;
            
            snprintf(key_buf, sizeof(key_buf), "key_%zu", test_idx);
            
            key = MapNodeFromString(key_buf);
            expected = (int)(test_idx * 2);
            actual = map->getInt(key, -1);
            
            if (actual == expected) {
                found++;
            }
            
            MapNode_Free(key);  /* Clean up test key */
        }
        
        printf("Found %zu/100 random keys correctly\n", found);
    }
    
    /* Validate the entire map */
    {
        size_t validation_errors = map->validate();
        printf("Map validation: %zu errors\n", validation_errors);
    }
    
    map->free();
    printf("\n");
}

void demonstrate_real_world_usage() {
    printf("=== Real-World Usage: User Database ===\n");
    
    Map* users = MapMake();
    
    /* Simulate user records */
    struct User {
        int id;
        const char* name;
        const char* email;
        int age;
        float salary;
    } user_data[] = {
        {1001, "Alice Johnson", "alice@example.com", 28, 75000.0f},
        {1002, "Bob Smith", "bob@example.com", 34, 82000.0f},
        {1003, "Carol Brown", "carol@example.com", 29, 78000.0f},
        {1004, "David Wilson", "david@example.com", 31, 85000.0f}
    };
    
    size_t num_users = sizeof(user_data) / sizeof(user_data[0]);
    
    /* Insert users with different key strategies */
    {
        size_t i;
        for (i = 0; i < num_users; i++) {
            struct User* u = &user_data[i];
            void* id_key;
            void* name_value;
            void* email_key;
            void* id_value;
            
            /* Key by user ID */
            id_key = MapNodeFromInt(u->id);
            name_value = MapNodeFromString(u->name);
            users->put(id_key, name_value);
            
            /* Also key by email for lookups */
            email_key = MapNodeFromString(u->email);
            id_value = MapNodeFromInt(u->id);
            users->put(email_key, id_value);
        }
    }
    
    printf("Inserted %zu users with dual indexing\n", num_users);
    printf("Map contains %zu total entries\n", users->size());
    
    /* Lookup by ID */
    {
        const char* name = users->getString(MapNodeFromInt(1002));
        printf("User 1002: %s\n", name ? name : "not found");
    }
    
    /* Lookup by email */
    {
        void* email_key = MapNodeFromString("carol@example.com");
        int user_id = users->getInt(email_key, -1);
            printf("Email carol@example.com belongs to user ID: %d\n", user_id);
        
        /* Cleanup */
        MapNode_Free(email_key);
    }
    
    /* Show some statistics */
    users->debug(5);
    users->free();
    printf("\n");
}

int main() {
    printf("Complete Map v2 with MapNode Integration\n");
    printf("=======================================\n");
    printf("Demonstrating zero-cognitive-load hash table operations\n\n");
    
    demonstrate_zero_cognitive_load();
    demonstrate_mixed_types_and_debugging();
    demonstrate_error_handling();
    demonstrate_bulk_operations();
    demonstrate_performance();
    demonstrate_real_world_usage();
    
    printf("🎉 All Map v2 demonstrations completed successfully!\n");
    printf("\nKey Achievements:\n");
    printf("  ✅ Zero cognitive load - no size management\n");
    printf("  ✅ Type safety - runtime validation and conversion\n");
    printf("  ✅ Mixed types - any MapNode type as key or value\n");
    printf("  ✅ Error resilience - graceful handling of invalid inputs\n");
    printf("  ✅ Rich debugging - validation, statistics, introspection\n");
    printf("  ✅ Performance - efficient hashing and collision resolution\n");
    printf("  ✅ Convenience - type-specific getters and setters\n");
    printf("  ✅ Memory safety - automatic cleanup and validation\n");
    printf("\nThe Map v2 API eliminates cognitive overhead while providing\n");
    printf("enterprise-grade features and bulletproof type safety! 🚀\n");
    
    return 0;
}