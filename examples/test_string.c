/**
 * Test program for the modular trampoline string class
 * 
 * This demonstrates using the string class as a separate library
 * that depends on the core trampoline library.
 */

#include <trampolines/string.h>
#include <stdio.h>

int main(void) {
    String* str;
    String* upper;
    String** parts;
    size_t count;
    size_t i;
    
    printf("=== Modular Trampoline String Test ===\n\n");
    
    /* Create a string */
    str = StringMake("Hello, World!");
    if (!str) {
        printf("Failed to create string\n");
        return 1;
    }
    
    printf("Original: '%s' (length: %zu)\n", str->cStr(), str->length());
    
    /* Test append */
    str->append(" How are you?");
    printf("After append: '%s'\n", str->cStr());
    
    /* Test uppercase transformation */
    upper = str->toUpperCase();
    printf("Uppercase: '%s'\n", upper->cStr());
    upper->free();
    
    /* Test splitting */
    str->set("apple,banana,cherry,date");
    printf("\nSplitting: '%s'\n", str->cStr());
    parts = str->split(",", &count);
    
    printf("Parts (%zu):\n", count);
    for (i = 0; i < count; i++) {
        printf("  [%zu]: '%s'\n", i, parts[i]->cStr());
    }
    
    /* Clean up split results */
    StringArray_Free(parts, count);
    
    /* Test contains */
    printf("\nContains 'banana': %s\n", 
           str->contains("banana") ? "yes" : "no");
    printf("Contains 'grape': %s\n", 
           str->contains("grape") ? "yes" : "no");
    
    /* Clean up */
    str->free();
    
    printf("\n=== Test completed successfully ===\n");
    return 0;
}