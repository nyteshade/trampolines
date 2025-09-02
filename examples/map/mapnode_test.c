#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <trampoline.h>

#include "mapnode.h"
#include "mapnode_impl.c"

void test_mapnode_creation() {
    printf("=== Testing MapNode Creation ===\n");
    
    // Test integer
    void* int_node = MapNodeFromInt(42);
    printf("Created int node: %p\n", int_node);
    assert(MapNode_IsValid(int_node));
    assert(MapNode_GetType(int_node) == MAPNODE_TYPE_INT);
    printf("Magic: 0x%x, Type: %d\n", MapNode_GetMagic(int_node), MapNode_GetType(int_node));
    
    // Test string
    void* str_node = MapNodeFromString("Hello World");
    printf("Created string node: %p\n", str_node);
    assert(MapNode_IsValid(str_node));
    assert(MapNode_GetType(str_node) == MAPNODE_TYPE_STRING);
    
    // Test double
    void* double_node = MapNodeFromDouble(3.14159);
    printf("Created double node: %p\n", double_node);
    assert(MapNode_IsValid(double_node));
    assert(MapNode_GetType(double_node) == MAPNODE_TYPE_DOUBLE);
    
    // Test invalid pointer
    char* invalid = "not a mapnode";
    assert(!MapNode_IsValid(invalid));
    printf("Invalid pointer correctly rejected\n");
    
    // Clean up
    MapNode_Free(int_node);
    MapNode_Free(str_node);
    MapNode_Free(double_node);
    
    printf("MapNode creation tests passed!\n\n");
}

void test_mapnode_interface() {
    printf("=== Testing MapNode Interface ===\n");
    
    // Create some nodes
    void* int_node = MapNodeFromInt(123);
    void* str_node = MapNodeFromString("test string");
    void* float_node = MapNodeFromFloat(45.67f);
    
    // Cast to interfaces
    MapNode* int_iface = MapNode_Cast(int_node);
    MapNode* str_iface = MapNode_Cast(str_node);
    MapNode* float_iface = MapNode_Cast(float_node);
    
    assert(int_iface != NULL);
    assert(str_iface != NULL);
    assert(float_iface != NULL);
    
    // Test type checking
    assert(int_iface->isInt());
    assert(!int_iface->isString());
    assert(!int_iface->isFloat());
    
    assert(str_iface->isString());
    assert(!str_iface->isInt());
    
    assert(float_iface->isFloat());
    assert(!float_iface->isString());
    
    // Test value retrieval
    assert(int_iface->asInt() == 123);
    assert(strcmp(str_iface->asString(), "test string") == 0);
    assert(float_iface->asFloat() == 45.67f);
    
    // Test type names
    printf("Int type name: %s\n", int_iface->typeName());
    printf("String type name: %s\n", str_iface->typeName());
    printf("Float type name: %s\n", float_iface->typeName());
    
    // Test sizes
    printf("Int size: %zu\n", int_iface->size());
    printf("String size: %zu\n", str_iface->size());
    printf("Float size: %zu\n", float_iface->size());
    
    // Clean up
    MapNode_Free(int_node);
    MapNode_Free(str_node);
    MapNode_Free(float_node);
    
    printf("MapNode interface tests passed!\n\n");
}

void test_mapnode_copying() {
    printf("=== Testing MapNode Copying ===\n");
    
    // Create original nodes
    void* orig_int = MapNodeFromInt(789);
    void* orig_str = MapNodeFromString("original");
    
    // Copy them
    void* copy_int = MapNodeCopy(orig_int);
    void* copy_str = MapNodeCopy(orig_str);
    
    assert(MapNode_IsValid(copy_int));
    assert(MapNode_IsValid(copy_str));
    
    // Verify copies are independent but equal
    assert(copy_int != orig_int);  // Different addresses
    assert(copy_str != orig_str);
    
    MapNode* orig_int_iface = MapNode_Cast(orig_int);
    MapNode* copy_int_iface = MapNode_Cast(copy_int);
    MapNode* orig_str_iface = MapNode_Cast(orig_str);
    MapNode* copy_str_iface = MapNode_Cast(copy_str);
    
    assert(orig_int_iface->asInt() == copy_int_iface->asInt());
    assert(strcmp(orig_str_iface->asString(), copy_str_iface->asString()) == 0);
    
    printf("Original int: %d, Copy int: %d\n", 
           orig_int_iface->asInt(), copy_int_iface->asInt());
    printf("Original str: %s, Copy str: %s\n",
           orig_str_iface->asString(), copy_str_iface->asString());
    
    // Test using trampoline copy method
    void* copy2_int = orig_int_iface->copy();
    assert(MapNode_IsValid(copy2_int));
    
    MapNode* copy2_int_iface = MapNode_Cast(copy2_int);
    assert(copy2_int_iface->asInt() == 789);
    
    // Clean up
    MapNode_Free(orig_int);
    MapNode_Free(orig_str);
    MapNode_Free(copy_int);
    MapNode_Free(copy_str);
    MapNode_Free(copy2_int);
    
    printf("MapNode copying tests passed!\n\n");
}

void test_mapnode_utilities() {
    printf("=== Testing MapNode Utilities ===\n");
    
    // Create test nodes
    void* int_node = MapNodeFromInt(42);
    void* str_node = MapNodeFromString("hello");
    void* float_node = MapNodeFromFloat(3.14f);
    
    // Test toString
    char buffer[256];
    
    int len1 = MapNode_ToString(int_node, buffer, sizeof(buffer));
    printf("Int toString: %s (length: %d)\n", buffer, len1);
    
    int len2 = MapNode_ToString(str_node, buffer, sizeof(buffer));
    printf("String toString: %s (length: %d)\n", buffer, len2);
    
    int len3 = MapNode_ToString(float_node, buffer, sizeof(buffer));
    printf("Float toString: %s (length: %d)\n", buffer, len3);
    
    // Test comparison
    void* int_node2 = MapNodeFromInt(42);
    void* int_node3 = MapNodeFromInt(99);
    
    int cmp1 = MapNode_Compare(int_node, int_node2);  // Should be 0 (equal)
    int cmp2 = MapNode_Compare(int_node, int_node3);  // Should be non-zero
    int cmp3 = MapNode_Compare(int_node, str_node);   // Different types
    
    printf("Compare 42 vs 42: %d\n", cmp1);
    printf("Compare 42 vs 99: %d\n", cmp2);
    printf("Compare int vs string: %d\n", cmp3);
    
    assert(cmp1 == 0);
    assert(cmp2 != 0);
    assert(cmp3 != 0);
    
    // Test hashing
    size_t hash1 = MapNode_Hash(int_node);
    size_t hash2 = MapNode_Hash(int_node2);
    size_t hash3 = MapNode_Hash(int_node3);
    size_t hash4 = MapNode_Hash(str_node);
    
    printf("Hash of int(42): %zu\n", hash1);
    printf("Hash of int(42) copy: %zu\n", hash2);
    printf("Hash of int(99): %zu\n", hash3);
    printf("Hash of string: %zu\n", hash4);
    
    assert(hash1 == hash2);  // Same values should hash the same
    assert(hash1 != hash3);  // Different values should hash differently
    
    // Clean up
    MapNode_Free(int_node);
    MapNode_Free(str_node);
    MapNode_Free(float_node);
    MapNode_Free(int_node2);
    MapNode_Free(int_node3);
    
    printf("MapNode utility tests passed!\n\n");
}

void test_memory_introspection() {
    printf("=== Testing Memory Introspection ===\n");
    
    void* node = MapNodeFromString("memory test");
    
    // Test direct memory access
    printf("Node pointer: %p\n", node);
    printf("Magic at offset %zu: 0x%x\n", sizeof(void*), MapNode_GetMagic(node));
    printf("Type at offset %zu: %d\n", sizeof(void*) + sizeof(uint32_t), MapNode_GetType(node));
    printf("Size: %zu\n", MapNode_GetSize(node));
    
    // Verify magic bytes manually
    const char* byte_ptr = (const char*)node;
    const uint32_t* magic_ptr = (const uint32_t*)(byte_ptr + sizeof(void*));
    const uint32_t* type_ptr = (const uint32_t*)(byte_ptr + sizeof(void*) + sizeof(uint32_t));
    
    printf("Manual magic read: 0x%x\n", *magic_ptr);
    printf("Manual type read: %d\n", *type_ptr);
    
    assert(*magic_ptr == MAPNODE_MAGIC_STRING);
    assert(*type_ptr == MAPNODE_TYPE_STRING);
    
    // Test validation on random memory
    char random_memory[32] = {0};
    assert(!MapNode_IsValid(random_memory));
    
    // Test validation on NULL
    assert(!MapNode_IsValid(NULL));
    
    MapNode_Free(node);
    
    printf("Memory introspection tests passed!\n\n");
}

void stress_test() {
    printf("=== Stress Testing MapNode ===\n");
    
    const int num_nodes = 1000;
    void* nodes[num_nodes];
    
    // Create many nodes
    printf("Creating %d MapNodes...\n", num_nodes);
    for (int i = 0; i < num_nodes; i++) {
        if (i % 4 == 0) {
            nodes[i] = MapNodeFromInt(i);
        } else if (i % 4 == 1) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "string_%d", i);
            nodes[i] = MapNodeFromString(buffer);
        } else if (i % 4 == 2) {
            nodes[i] = MapNodeFromFloat((float)i * 1.5f);
        } else {
            nodes[i] = MapNodeFromDouble((double)i * 2.7);
        }
        
        assert(MapNode_IsValid(nodes[i]));
    }
    
    // Validate all nodes
    printf("Validating all nodes...\n");
    for (int i = 0; i < num_nodes; i++) {
        assert(MapNode_IsValid(nodes[i]));
        
        MapNode* iface = MapNode_Cast(nodes[i]);
        assert(iface != NULL);
        
        // Verify type and value
        if (i % 4 == 0) {
            assert(iface->isInt());
            assert(iface->asInt() == i);
        } else if (i % 4 == 1) {
            assert(iface->isString());
            char expected[32];
            snprintf(expected, sizeof(expected), "string_%d", i);
            assert(strcmp(iface->asString(), expected) == 0);
        } else if (i % 4 == 2) {
            assert(iface->isFloat());
            assert(iface->asFloat() == (float)i * 1.5f);
        } else {
            assert(iface->isDouble());
            assert(iface->asDouble() == (double)i * 2.7);
        }
    }
    
    // Free all nodes
    printf("Freeing all nodes...\n");
    for (int i = 0; i < num_nodes; i++) {
        MapNode_Free(nodes[i]);
    }
    
    printf("Stress test completed successfully!\n\n");
}

int main() {
    printf("MapNode Comprehensive Test Suite\n");
    printf("================================\n\n");
    
    test_mapnode_creation();
    test_mapnode_interface();
    test_mapnode_copying();
    test_mapnode_utilities();
    test_memory_introspection();
    stress_test();
    
    printf("🎉 All MapNode tests passed successfully!\n");
    printf("MapNode system is ready for integration with Map.\n");
    
    return 0;
}