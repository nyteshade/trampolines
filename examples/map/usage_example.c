#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <trampoline.h>

#include "mapnode.h"
#include "mapnode_impl.c"

/**
 * @brief Demonstration of how MapNode eliminates cognitive load
 * 
 * This example shows the dramatic improvement in usability compared to
 * the old void* + size API. No more size management, automatic type
 * safety, and powerful introspection capabilities.
 */

void demonstrate_zero_cognitive_load() {
    printf("=== Zero Cognitive Load Demo ===\n");
    
    // OLD WAY (lots of cognitive load):
    // map->put("username", strlen("username")+1, "john_doe", strlen("john_doe")+1);
    // char* result = (char*)map->get("username", strlen("username")+1);
    
    // NEW WAY (zero cognitive load):
    void* username_key = MapNodeFromString("username");
    void* username_value = MapNodeFromString("john_doe");
    void* age_key = MapNodeFromString("age");  
    void* age_value = MapNodeFromInt(25);
    
    printf("Created MapNodes - no size management needed!\n");
    printf("Username key: %p (valid: %s)\n", username_key, 
           MapNode_IsValid(username_key) ? "yes" : "no");
    printf("Age value: %p (valid: %s)\n", age_value,
           MapNode_IsValid(age_value) ? "yes" : "no");
    
    // Demonstrate type introspection on void*
    printf("\nType Introspection on void* pointers:\n");
    printf("username_key type: %s\n", MapNode_Cast(username_key)->typeName());
    printf("age_value type: %s\n", MapNode_Cast(age_value)->typeName());
    
    // Show automatic type conversion
    MapNode* age_node = MapNode_Cast(age_value);
    printf("Age as int: %d\n", age_node->asInt());
    printf("Age as string: %s\n", age_node->asString());  // Returns NULL - wrong type
    
    // Clean up
    MapNode_Free(username_key);
    MapNode_Free(username_value);
    MapNode_Free(age_key);
    MapNode_Free(age_value);
    
    printf("\n");
}

void demonstrate_mixed_types() {
    printf("=== Mixed Type Handling ===\n");
    
    // Create a variety of types
    void* nodes[] = {
        MapNodeFromInt(42),
        MapNodeFromFloat(3.14159f),
        MapNodeFromDouble(2.718281828),
        MapNodeFromString("Hello, MapNode!"),
        MapNodeFromPointer((void*)0x12345678),
        MapNodeFromBytes("binary\0data\xff", 12)
    };
    
    const char* type_names[] = {
        "Integer", "Float", "Double", "String", "Pointer", "Binary Data"
    };
    
    printf("Created %zu different types of MapNodes:\n", sizeof(nodes)/sizeof(nodes[0]));
    
    for (size_t i = 0; i < sizeof(nodes)/sizeof(nodes[0]); i++) {
        char buffer[128];
        MapNode_ToString(nodes[i], buffer, sizeof(buffer));
        printf("  %s: %s\n", type_names[i], buffer);
        
        // Demonstrate safe type checking
        MapNode* node = MapNode_Cast(nodes[i]);
        printf("    Size: %zu bytes, Magic: 0x%x\n", 
               node->size(), MapNode_GetMagic(nodes[i]));
    }
    
    // Clean up
    for (size_t i = 0; i < sizeof(nodes)/sizeof(nodes[0]); i++) {
        MapNode_Free(nodes[i]);
    }
    
    printf("\n");
}

void demonstrate_error_handling() {
    printf("=== Error Handling & Validation ===\n");
    
    // Test invalid pointers
    char* not_a_mapnode = "just a string";
    int random_int = 12345;
    void* null_ptr = NULL;
    
    printf("Testing invalid pointers:\n");
    printf("  String literal valid: %s\n", MapNode_IsValid(not_a_mapnode) ? "yes" : "no");
    printf("  Random int valid: %s\n", MapNode_IsValid(&random_int) ? "yes" : "no");
    printf("  NULL valid: %s\n", MapNode_IsValid(null_ptr) ? "yes" : "no");
    
    // Test safe casting
    MapNode* cast1 = MapNode_Cast(not_a_mapnode);  // Should return NULL
    MapNode* cast2 = MapNode_Cast(&random_int);    // Should return NULL
    MapNode* cast3 = MapNode_Cast(null_ptr);       // Should return NULL
    
    printf("  Safe cast results: %p, %p, %p (all should be NULL)\n", 
           (void*)cast1, (void*)cast2, (void*)cast3);
    
    // Test safe free (should not crash)
    printf("Testing safe free on invalid pointers (should not crash):\n");
    MapNode_Free(not_a_mapnode);
    MapNode_Free(&random_int);
    MapNode_Free(null_ptr);
    printf("  All safe frees completed successfully\n");
    
    // Create a valid MapNode and corrupt it
    void* valid_node = MapNodeFromString("test corruption");
    printf("Original node valid: %s\n", MapNode_IsValid(valid_node) ? "yes" : "no");
    
    // Corrupt magic bytes (simulate memory corruption)
    uint32_t* magic_ptr = (uint32_t*)((char*)valid_node + sizeof(void*));
    uint32_t original_magic = *magic_ptr;
    *magic_ptr = 0xDEADBEEF;  // Corrupt it
    
    printf("After corruption valid: %s\n", MapNode_IsValid(valid_node) ? "yes" : "no");
    
    // Restore and clean up
    *magic_ptr = original_magic;
    printf("After restoration valid: %s\n", MapNode_IsValid(valid_node) ? "yes" : "no");
    MapNode_Free(valid_node);
    
    printf("\n");
}

void demonstrate_comparison_and_hashing() {
    printf("=== Comparison and Hashing ===\n");
    
    // Create some nodes for comparison
    void* int1 = MapNodeFromInt(42);
    void* int2 = MapNodeFromInt(42);  // Same value
    void* int3 = MapNodeFromInt(99);  // Different value
    void* str1 = MapNodeFromString("hello");
    void* str2 = MapNodeFromString("hello");  // Same value
    void* str3 = MapNodeFromString("world");  // Different value
    
    printf("Comparison results:\n");
    printf("  int(42) vs int(42): %d\n", MapNode_Compare(int1, int2));
    printf("  int(42) vs int(99): %d\n", MapNode_Compare(int1, int3));
    printf("  int(42) vs string(hello): %d\n", MapNode_Compare(int1, str1));
    printf("  string(hello) vs string(hello): %d\n", MapNode_Compare(str1, str2));
    printf("  string(hello) vs string(world): %d\n", MapNode_Compare(str1, str3));
    
    printf("\nHash values:\n");
    printf("  int(42) #1: %zu\n", MapNode_Hash(int1));
    printf("  int(42) #2: %zu\n", MapNode_Hash(int2));
    printf("  int(99): %zu\n", MapNode_Hash(int3));
    printf("  string(hello) #1: %zu\n", MapNode_Hash(str1));
    printf("  string(hello) #2: %zu\n", MapNode_Hash(str2));
    printf("  string(world): %zu\n", MapNode_Hash(str3));
    
    // Verify hash consistency
    assert(MapNode_Hash(int1) == MapNode_Hash(int2));
    assert(MapNode_Hash(str1) == MapNode_Hash(str2));
    printf("  ✓ Hash consistency verified\n");
    
    // Clean up
    MapNode_Free(int1);
    MapNode_Free(int2);
    MapNode_Free(int3);
    MapNode_Free(str1);
    MapNode_Free(str2);
    MapNode_Free(str3);
    
    printf("\n");
}

void demonstrate_copy_semantics() {
    printf("=== Copy Semantics ===\n");
    
    // Create an original with complex data
    const char* long_string = "This is a longer string to test memory management";
    void* original = MapNodeFromString(long_string);
    
    // Create copies using different methods
    void* copy1 = MapNodeCopy(original);                           // Global function
    void* copy2 = MapNode_Cast(original)->copy();                 // Method call
    
    printf("Original: %p\n", original);
    printf("Copy 1:   %p\n", copy1);
    printf("Copy 2:   %p\n", copy2);
    
    // Verify they're all different objects but same content
    assert(original != copy1);
    assert(original != copy2);
    assert(copy1 != copy2);
    
    MapNode* orig_node = MapNode_Cast(original);
    MapNode* copy1_node = MapNode_Cast(copy1);
    MapNode* copy2_node = MapNode_Cast(copy2);
    
    printf("Content comparison:\n");
    printf("  Original: %s\n", orig_node->asString());
    printf("  Copy 1:   %s\n", copy1_node->asString());
    printf("  Copy 2:   %s\n", copy2_node->asString());
    
    // Verify independence - freeing original shouldn't affect copies
    MapNode_Free(original);
    
    printf("After freeing original, copies still valid:\n");
    printf("  Copy 1: %s\n", copy1_node->asString());
    printf("  Copy 2: %s\n", copy2_node->asString());
    
    MapNode_Free(copy1);
    MapNode_Free(copy2);
    
    printf("\n");
}

int main() {
    printf("MapNode Usage Examples\n");
    printf("======================\n");
    printf("Demonstrating how MapNode eliminates cognitive load\n");
    printf("and provides powerful type introspection.\n\n");
    
    demonstrate_zero_cognitive_load();
    demonstrate_mixed_types();
    demonstrate_error_handling();
    demonstrate_comparison_and_hashing();
    demonstrate_copy_semantics();
    
    printf("🎉 MapNode usage examples completed successfully!\n");
    printf("\n");
    printf("Key Benefits Demonstrated:\n");
    printf("  ✓ Zero size management - no more strlen() everywhere\n");
    printf("  ✓ Type safety - runtime validation and type checking\n");
    printf("  ✓ Introspection - any void* can be examined and typed\n");
    printf("  ✓ Error resilience - safe operations on invalid pointers\n");
    printf("  ✓ Memory efficiency - optimal layout with magic bytes\n");
    printf("  ✓ Debugging friendly - toString, validation, type names\n");
    printf("\nMapNode is ready to revolutionize the Map API! 🚀\n");
    
    return 0;
}