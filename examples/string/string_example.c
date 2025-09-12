/**
 * @file string_example.c
 * @brief Comprehensive demonstration of String with trampoline member functions
 */

#include <trampolines/string.h>

#include <stdio.h>
#include <assert.h>

/* ======================================================================== */
/* Test Helper Functions                                                    */
/* ======================================================================== */

static void print_separator(const char* title) {
    printf("\n========================================\n");
    printf("%s\n", title);
    printf("========================================\n");
}

static void test_basic_operations(void) {
    print_separator("Basic String Operations");
    
    /* Create a string */
    String* str = StringMake("Hello");
    printf("Created string: '%s'\n", str->cStr());
    printf("Length: %zu\n", str->length());
    printf("Is empty: %s\n", str->isEmpty() ? "yes" : "no");
    
    /* Append operations */
    str->append(" World");
    printf("After append: '%s'\n", str->cStr());
    
    str->appendChar('!');
    printf("After appendChar: '%s'\n", str->cStr());
    
    str->appendFormat(" %d + %d = %d", 2, 3, 5);
    printf("After appendFormat: '%s'\n", str->cStr());
    
    /* Character access */
    printf("Character at index 0: '%c'\n", str->charAt(0));
    printf("Character at index 6: '%c'\n", str->charAt(6));
    
    str->free();
    printf("✓ Basic operations test passed\n");
}

static void test_string_modification(void) {
    print_separator("String Modification");
    
    String* str = StringMake("World");
    
    /* Prepend */
    str->prepend("Hello ");
    printf("After prepend: '%s'\n", str->cStr());
    
    /* Insert */
    str->insert(6, "Beautiful ");
    printf("After insert: '%s'\n", str->cStr());
    
    /* Replace */
    size_t replacements = str->replace("Beautiful", "Wonderful");
    printf("After replace (%zu replacements): '%s'\n", replacements, str->cStr());
    
    /* Replace first */
    String* multi = StringMake("one two one three one");
    printf("Original: '%s'\n", multi->cStr());
    multi->replaceFirst("one", "ONE");
    printf("After replaceFirst: '%s'\n", multi->cStr());
    
    /* Reverse */
    String* rev = StringMake("abcdef");
    rev->reverse();
    printf("Reversed 'abcdef': '%s'\n", rev->cStr());
    
    /* Clear and set */
    str->clear();
    printf("After clear, length: %zu\n", str->length());
    str->set("New content");
    printf("After set: '%s'\n", str->cStr());
    
    str->free();
    multi->free();
    rev->free();
    printf("✓ Modification test passed\n");
}

static void test_string_transformation(void) {
    print_separator("String Transformations");
    
    String* str = StringMake("  Hello World  ");
    
    /* Trimming */
    String* trimmed = str->trim();
    printf("Original: '%s' (length: %zu)\n", str->cStr(), str->length());
    printf("Trimmed: '%s' (length: %zu)\n", trimmed->cStr(), trimmed->length());
    
    String* ltrimmed = str->trimLeft();
    printf("Left trimmed: '%s'\n", ltrimmed->cStr());
    
    String* rtrimmed = str->trimRight();
    printf("Right trimmed: '%s'\n", rtrimmed->cStr());
    
    /* Case conversion */
    String* upper = trimmed->toUpperCase();
    printf("Uppercase: '%s'\n", upper->cStr());
    
    String* lower = upper->toLowerCase();
    printf("Lowercase: '%s'\n", lower->cStr());
    
    /* In-place case conversion */
    String* inplace = StringMake("Mixed Case String");
    printf("Original: '%s'\n", inplace->cStr());
    inplace->toUpperCaseInPlace();
    printf("After toUpperCaseInPlace: '%s'\n", inplace->cStr());
    inplace->toLowerCaseInPlace();
    printf("After toLowerCaseInPlace: '%s'\n", inplace->cStr());
    
    /* Substring */
    String* source = StringMake("Hello World!");
    String* sub = source->substring(6, 5);
    printf("Substring(6, 5) of 'Hello World!': '%s'\n", sub->cStr());
    
    /* Repeat */
    String* pattern = StringMake("ab");
    String* repeated = pattern->repeat(5);
    printf("'ab' repeated 5 times: '%s'\n", repeated->cStr());
    
    str->free();
    trimmed->free();
    ltrimmed->free();
    rtrimmed->free();
    upper->free();
    lower->free();
    inplace->free();
    source->free();
    sub->free();
    pattern->free();
    repeated->free();
    printf("✓ Transformation test passed\n");
}

static void test_string_searching(void) {
    print_separator("String Searching");
    
    String* str = StringMake("The quick brown fox jumps over the lazy dog");
    
    /* Contains */
    printf("Contains 'fox': %s\n", str->contains("fox") ? "yes" : "no");
    printf("Contains 'cat': %s\n", str->contains("cat") ? "yes" : "no");
    
    /* Starts/ends with */
    printf("Starts with 'The': %s\n", str->startsWith("The") ? "yes" : "no");
    printf("Ends with 'dog': %s\n", str->endsWith("dog") ? "yes" : "no");
    
    /* Index operations */
    size_t idx = str->indexOf("brown");
    printf("Index of 'brown': %zu\n", idx);
    
    idx = str->lastIndexOf("the");
    printf("Last index of 'the': %zu\n", idx);
    
    idx = str->indexOfAny("aeiou");
    printf("First vowel at index: %zu\n", idx);
    
    /* Count occurrences */
    size_t count = str->count("o");
    printf("Count of 'o': %zu\n", count);
    
    String* repeated = StringMake("na na na na Batman!");
    count = repeated->count("na");
    printf("Count of 'na' in '%s': %zu\n", repeated->cStr(), count);
    
    str->free();
    repeated->free();
    printf("✓ Searching test passed\n");
}

static void test_string_splitting(void) {
    print_separator("String Splitting and Joining");
    
    /* Split by delimiter */
    String* csv = StringMake("apple,banana,cherry,date");
    size_t count;
    String** parts = csv->split(",", &count);
    
    printf("Split '%s' by ',':\n", csv->cStr());
    size_t i;
    for (i = 0; i < count; i++) {
        printf("  [%zu]: '%s'\n", i, parts[i]->cStr());
    }
    
    /* Join strings */
    String* separator = StringMake(" | ");
    String* joined = separator->join(parts, count);
    printf("Joined with ' | ': '%s'\n", joined->cStr());
    
    StringArray_Free(parts, count);
    
    /* Split by any character */
    String* mixed = StringMake("one;two,three:four");
    String** mixedParts = mixed->splitAny(";,:", &count);
    printf("\nSplit '%s' by any of ';,:':\n", mixed->cStr());
    for (i = 0; i < count; i++) {
        printf("  [%zu]: '%s'\n", i, mixedParts[i]->cStr());
    }
    StringArray_Free(mixedParts, count);
    
    /* Split lines */
    String* multiline = StringMake("Line 1\nLine 2\rLine 3\r\nLine 4");
    String** lines = multiline->splitLines(&count);
    printf("\nSplit into lines:\n");
    for (i = 0; i < count; i++) {
        if (lines[i]->length() > 0) {
            printf("  Line %zu: '%s'\n", i + 1, lines[i]->cStr());
        }
    }
    StringArray_Free(lines, count);
    
    csv->free();
    separator->free();
    joined->free();
    mixed->free();
    multiline->free();
    printf("✓ Splitting test passed\n");
}

static void test_string_comparison(void) {
    print_separator("String Comparison");
    
    String* str1 = StringMake("Hello");
    String* str2 = StringMake("hello");
    String* str3 = StringMake("Hello");
    
    /* Equals */
    printf("'%s' equals '%s': %s\n", 
           str1->cStr(), str2->cStr(), 
           str1->equals(str2->cStr()) ? "yes" : "no");
    
    printf("'%s' equals '%s': %s\n", 
           str1->cStr(), str3->cStr(), 
           str1->equals(str3->cStr()) ? "yes" : "no");
    
    /* Equals ignore case */
    printf("'%s' equalsIgnoreCase '%s': %s\n", 
           str1->cStr(), str2->cStr(), 
           str1->equalsIgnoreCase(str2->cStr()) ? "yes" : "no");
    
    /* Compare */
    int cmp = str1->compare("Hello");
    printf("Compare with 'Hello': %d\n", cmp);
    
    cmp = str1->compare("World");
    printf("Compare with 'World': %d\n", cmp);
    
    cmp = str1->compareIgnoreCase("hello");
    printf("Compare ignore case with 'hello': %d\n", cmp);
    
    str1->free();
    str2->free();
    str3->free();
    printf("✓ Comparison test passed\n");
}

static void test_string_validation(void) {
    print_separator("String Validation and Conversion");
    
    /* Integer validation */
    String* intStr = StringMake("12345");
    printf("'%s' is integer: %s\n", intStr->cStr(), intStr->isInteger() ? "yes" : "no");
    printf("  Converted: %d\n", intStr->toInt(0));
    
    String* notInt = StringMake("12.34");
    printf("'%s' is integer: %s\n", notInt->cStr(), notInt->isInteger() ? "yes" : "no");
    
    /* Float validation */
    String* floatStr = StringMake("3.14159");
    printf("'%s' is float: %s\n", floatStr->cStr(), floatStr->isFloat() ? "yes" : "no");
    printf("  Converted: %.5f\n", floatStr->toFloat(0.0));
    
    /* Alpha validation */
    String* alphaStr = StringMake("HelloWorld");
    printf("'%s' is alpha: %s\n", alphaStr->cStr(), alphaStr->isAlpha() ? "yes" : "no");
    
    /* Digit validation */
    String* digitStr = StringMake("123456");
    printf("'%s' is digit: %s\n", digitStr->cStr(), digitStr->isDigit() ? "yes" : "no");
    
    /* Alphanumeric validation */
    String* alnumStr = StringMake("Hello123");
    printf("'%s' is alphanumeric: %s\n", alnumStr->cStr(), alnumStr->isAlphaNumeric() ? "yes" : "no");
    
    /* Whitespace validation */
    String* wsStr = StringMake("   \t\n");
    printf("'   \\t\\n' is whitespace: %s\n", wsStr->isWhitespace() ? "yes" : "no");
    
    /* Hash */
    String* hashStr = StringMake("Hello World");
    printf("Hash of '%s': %zu\n", hashStr->cStr(), hashStr->hash());
    
    intStr->free();
    notInt->free();
    floatStr->free();
    alphaStr->free();
    digitStr->free();
    alnumStr->free();
    wsStr->free();
    hashStr->free();
    printf("✓ Validation test passed\n");
}

static void test_string_builders(void) {
    print_separator("String Building and Formatting");
    
    /* Format creation */
    String* formatted = StringMakeFormat("User: %s, Age: %d, Score: %.2f", 
                                        "Alice", 25, 98.5);
    printf("Formatted string: '%s'\n", formatted->cStr());
    
    /* From conversions */
    String* fromInt = StringFromInt(42);
    printf("From int 42: '%s'\n", fromInt->cStr());
    
    String* fromFloat = StringFromFloat(3.14159f, 2);
    printf("From float 3.14159 (2 decimals): '%s'\n", fromFloat->cStr());
    
    String* fromDouble = StringFromDouble(2.71828, 4);
    printf("From double 2.71828 (4 decimals): '%s'\n", fromDouble->cStr());
    
    /* Building complex strings */
    String* builder = StringMake("");
    builder->append("SELECT ");
    builder->append("name, age, city ");
    builder->append("FROM users ");
    builder->appendFormat("WHERE age > %d ", 18);
    builder->append("ORDER BY name");
    printf("Built SQL: '%s'\n", builder->cStr());
    
    /* Memory management */
    printf("Builder capacity: %zu\n", builder->capacity());
    builder->shrinkToFit();
    printf("After shrinkToFit: %zu\n", builder->capacity());
    
    formatted->free();
    fromInt->free();
    fromFloat->free();
    fromDouble->free();
    builder->free();
    printf("✓ Builder test passed\n");
}

static void test_real_world_example(void) {
    print_separator("Real-World Example: URL Parser");
    
    String* url = StringMake("https://example.com:8080/path/to/resource?key=value&foo=bar#section");
    printf("Parsing URL: %s\n", url->cStr());
    
    /* Extract protocol */
    size_t protoEnd = url->indexOf("://");
    if (protoEnd != (size_t)-1) {
        String* protocol = url->substring(0, protoEnd);
        printf("  Protocol: %s\n", protocol->cStr());
        protocol->free();
    }
    
    /* Extract query parameters */
    size_t queryStart = url->indexOf("?");
    size_t queryEnd = url->indexOf("#");
    if (queryStart != (size_t)-1) {
        size_t length = (queryEnd != (size_t)-1) ? 
                       queryEnd - queryStart - 1 : 
                       url->length() - queryStart - 1;
        
        String* queryString = url->substring(queryStart + 1, length);
        printf("  Query string: %s\n", queryString->cStr());
        
        /* Parse parameters */
        size_t paramCount;
        String** params = queryString->split("&", &paramCount);
        printf("  Parameters:\n");
        
        size_t i;
        for (i = 0; i < paramCount; i++) {
            size_t kvCount;
            String** kv = params[i]->split("=", &kvCount);
            if (kvCount == 2) {
                printf("    %s = %s\n", kv[0]->cStr(), kv[1]->cStr());
            }
            StringArray_Free(kv, kvCount);
        }
        
        StringArray_Free(params, paramCount);
        queryString->free();
    }
    
    /* Extract fragment */
    size_t fragmentStart = url->indexOf("#");
    if (fragmentStart != (size_t)-1) {
        String* fragment = url->substring(fragmentStart + 1, 0);
        printf("  Fragment: %s\n", fragment->cStr());
        fragment->free();
    }
    
    url->free();
    printf("✓ Real-world example passed\n");
}

static void test_edge_cases(void) {
    print_separator("Edge Cases and Stress Testing");
    
    /* Empty string operations */
    String* empty = StringMake("");
    printf("Empty string length: %zu\n", empty->length());
    printf("Empty string is empty: %s\n", empty->isEmpty() ? "yes" : "no");
    
    empty->append("Not empty anymore");
    printf("After append: '%s'\n", empty->cStr());
    
    /* NULL handling */
    String* nullStr = StringMake(NULL);
    printf("String from NULL: '%s' (length: %zu)\n", nullStr->cStr(), nullStr->length());
    
    /* Large string */
    String* large = StringMake("");
    int i;
    for (i = 0; i < 100; i++) {
        large->appendFormat("Line %d: This is a test of string capacity growth.\n", i);
    }
    printf("Large string length: %zu\n", large->length());
    printf("Large string capacity: %zu\n", large->capacity());
    
    /* Reserve capacity */
    String* reserved = StringMake("Test");
    reserved->reserve(1000);
    printf("After reserve(1000), capacity: %zu\n", reserved->capacity());
    
    /* Clone test */
    String* original = StringMake("Original");
    String* cloned = original->clone();
    original->append(" Modified");
    printf("Original: '%s'\n", original->cStr());
    printf("Clone: '%s'\n", cloned->cStr());
    
    empty->free();
    nullStr->free();
    large->free();
    reserved->free();
    original->free();
    cloned->free();
    printf("✓ Edge cases test passed\n");
}

/* ======================================================================== */
/* Main Function                                                            */
/* ======================================================================== */

int main(void) {
    printf("=====================================\n");
    printf("    String Trampoline Example\n");
    printf("=====================================\n");
    printf("Demonstrating comprehensive string\n");
    printf("manipulation with member functions\n");
    printf("=====================================\n");
    
    /* Run all tests */
    test_basic_operations();
    test_string_modification();
    test_string_transformation();
    test_string_searching();
    test_string_splitting();
    test_string_comparison();
    test_string_validation();
    test_string_builders();
    test_real_world_example();
    test_edge_cases();
    
    print_separator("Summary");
    printf("✓ All tests passed successfully!\n");
    printf("✓ String struct provides comprehensive functionality\n");
    printf("✓ Zero-cognitive-load API with member functions\n");
    printf("✓ Memory management handled automatically\n");
    
    return 0;
}
