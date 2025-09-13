/**
 * @file simple_string_test.c
 * @brief Simple demonstration of String's zero-cognitive-load API
 */

#include <trampolines/string.h>

#include <stdio.h>

int main(void) {
    /* Create a string - as simple as it gets */
    String* name = StringMake("John Doe");
    
    /* Use it like an object - no need to pass 'name' as first parameter */
    printf("Original name: %s\n", name->cStr());
    printf("Length: %zu characters\n", name->length());
    
    /* Transform without hassle */
    String* upperName = name->toUpperCase();
    printf("Uppercase: %s\n", upperName->cStr());
    
    /* Build strings naturally */
    String* greeting = StringMake("Hello, ");
    greeting->append(name->cStr());
    greeting->append("! Welcome to the ");
    greeting->appendFormat("year %d", 2025);
    printf("Greeting: %s\n", greeting->cStr());
    
    /* Search operations are intuitive */
    if (greeting->contains("Welcome")) {
        printf("Found 'Welcome' in greeting\n");
    }
    
    /* Parse and manipulate */
    String* email = StringMake("  john.doe@example.com  ");
    String* trimmed = email->trim();
    printf("Email: '%s' -> '%s'\n", email->cStr(), trimmed->cStr());
    
    /* Split naturally */
    size_t parts;
    String** nameParts = trimmed->split("@", &parts);
    if (parts == 2) {
        printf("Username: %s\n", nameParts[0]->cStr());
        printf("Domain: %s\n", nameParts[1]->cStr());
    }
    
    /* Clean up is simple */
    StringArray_Free(nameParts, parts);
    name->free();
    upperName->free();
    greeting->free();
    email->free();
    trimmed->free();
    
    printf("\n✓ Simple string test completed!\n");
    return 0;
}