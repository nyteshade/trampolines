/**
 * @file string_demo_c89.c
 * @brief Fully C89-compliant demonstration of String with trampoline member functions
 */

#include <trampolines/string.h>

#include <stdio.h>

int main(void) {
    /* All variables declared at the beginning for C89 compliance */
    String* str;
    String* upper;
    String* trimmed;
    String* greeting;
    String** parts;
    size_t count;
    size_t i;

    printf("=====================================\n");
    printf("    String Trampoline Demo (C89)\n");
    printf("=====================================\n\n");

    /* Basic operations */
    printf("=== Basic Operations ===\n");
    str = StringMake("Hello World");
    printf("String: '%s'\n", str->cStr());
    printf("Length: %lu\n", (unsigned long)str->length());
    printf("Is empty: %s\n", str->isEmpty() ? "yes" : "no");
    
    /* Modification */
    str->append(" - Modified");
    printf("After append: '%s'\n", str->cStr());
    
    /* Transformation */
    printf("\n=== Transformations ===\n");
    upper = str->toUpperCase();
    printf("Uppercase: '%s'\n", upper->cStr());
    
    /* Trimming */
    str->set("  Spaces Around  ");
    trimmed = str->trim();
    printf("Original: '%s' (len=%lu)\n", str->cStr(), (unsigned long)str->length());
    printf("Trimmed: '%s' (len=%lu)\n", trimmed->cStr(), (unsigned long)trimmed->length());
    
    /* Searching */
    printf("\n=== Searching ===\n");
    greeting = StringMake("The quick brown fox");
    printf("String: '%s'\n", greeting->cStr());
    printf("Contains 'quick': %s\n", greeting->contains("quick") ? "yes" : "no");
    printf("Index of 'brown': %lu\n", (unsigned long)greeting->indexOf("brown"));
    
    /* Splitting */
    printf("\n=== Splitting ===\n");
    str->set("apple,banana,cherry");
    printf("CSV: '%s'\n", str->cStr());
    parts = str->split(",", &count);
    for (i = 0; i < count; i++) {
        printf("  Part %lu: '%s'\n", (unsigned long)(i + 1), parts[i]->cStr());
    }
    StringArray_Free(parts, count);
    
    /* Building */
    printf("\n=== Building ===\n");
    str->clear();
    str->append("Built ");
    str->append("piece ");
    str->append("by ");
    str->append("piece!");
    printf("Built string: '%s'\n", str->cStr());
    
    /* Clean up */
    str->free();
    upper->free();
    trimmed->free();
    greeting->free();
    
    printf("\n✓ Demo completed successfully!\n");
    return 0;
}