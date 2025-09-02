/**
 * @file string_performance.c
 * @brief Performance testing for String with trampolines
 */

#include "string.h"
#include "string_impl.c"
#include <stdio.h>
#include <time.h>

#define ITERATIONS 10000
#define LARGE_STRING_SIZE 1000

static double get_time_diff(clock_t start, clock_t end) {
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

static void test_append_performance() {
    clock_t start, end;
    int i;
    
    printf("Append Performance (%d iterations):\n", ITERATIONS);
    
    /* Test single character append */
    start = clock();
    String* str1 = StringMake("");
    for (i = 0; i < ITERATIONS; i++) {
        str1->appendChar('x');
    }
    end = clock();
    printf("  appendChar: %.4f seconds\n", get_time_diff(start, end));
    str1->free();
    
    /* Test string append */
    start = clock();
    String* str2 = StringMake("");
    for (i = 0; i < ITERATIONS; i++) {
        str2->append("test");
    }
    end = clock();
    printf("  append: %.4f seconds\n", get_time_diff(start, end));
    str2->free();
    
    /* Test format append */
    start = clock();
    String* str3 = StringMake("");
    for (i = 0; i < ITERATIONS / 10; i++) {
        str3->appendFormat("Item %d, ", i);
    }
    end = clock();
    printf("  appendFormat: %.4f seconds\n", get_time_diff(start, end));
    str3->free();
}

static void test_search_performance() {
    clock_t start, end;
    int i;
    
    printf("\nSearch Performance (%d iterations):\n", ITERATIONS);
    
    /* Create a large string for searching */
    String* haystack = StringMake("");
    for (i = 0; i < 100; i++) {
        haystack->append("The quick brown fox jumps over the lazy dog. ");
    }
    
    /* Test contains */
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        haystack->contains("fox");
    }
    end = clock();
    printf("  contains: %.4f seconds\n", get_time_diff(start, end));
    
    /* Test indexOf */
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        haystack->indexOf("lazy");
    }
    end = clock();
    printf("  indexOf: %.4f seconds\n", get_time_diff(start, end));
    
    /* Test count */
    start = clock();
    for (i = 0; i < ITERATIONS / 10; i++) {
        haystack->count("the");
    }
    end = clock();
    printf("  count: %.4f seconds\n", get_time_diff(start, end));
    
    haystack->free();
}

static void test_transformation_performance() {
    clock_t start, end;
    int i;
    
    printf("\nTransformation Performance (%d iterations):\n", ITERATIONS);
    
    String* source = StringMake("The Quick Brown Fox Jumps Over The Lazy Dog");
    
    /* Test case conversion */
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        String* upper = source->toUpperCase();
        upper->free();
    }
    end = clock();
    printf("  toUpperCase: %.4f seconds\n", get_time_diff(start, end));
    
    /* Test trimming */
    String* padded = StringMake("   text with spaces   ");
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        String* trimmed = padded->trim();
        trimmed->free();
    }
    end = clock();
    printf("  trim: %.4f seconds\n", get_time_diff(start, end));
    
    /* Test substring */
    start = clock();
    for (i = 0; i < ITERATIONS; i++) {
        String* sub = source->substring(4, 10);
        sub->free();
    }
    end = clock();
    printf("  substring: %.4f seconds\n", get_time_diff(start, end));
    
    source->free();
    padded->free();
}

static void test_split_join_performance() {
    clock_t start, end;
    int i;
    
    printf("\nSplit/Join Performance:\n");
    
    String* csv = StringMake("one,two,three,four,five,six,seven,eight,nine,ten");
    
    /* Test split */
    start = clock();
    for (i = 0; i < ITERATIONS / 10; i++) {
        size_t count;
        String** parts = csv->split(",", &count);
        StringArray_Free(parts, count);
    }
    end = clock();
    printf("  split: %.4f seconds (%d iterations)\n", 
           get_time_diff(start, end), ITERATIONS / 10);
    
    /* Test join */
    size_t count;
    String** parts = csv->split(",", &count);
    String* separator = StringMake("|");
    
    start = clock();
    for (i = 0; i < ITERATIONS / 10; i++) {
        String* joined = separator->join(parts, count);
        joined->free();
    }
    end = clock();
    printf("  join: %.4f seconds (%d iterations)\n", 
           get_time_diff(start, end), ITERATIONS / 10);
    
    StringArray_Free(parts, count);
    csv->free();
    separator->free();
}

static void test_memory_efficiency() {
    printf("\nMemory Efficiency Test:\n");
    
    /* Test capacity growth */
    String* str = StringMake("");
    printf("  Initial capacity: %zu\n", str->capacity());
    
    int i;
    for (i = 0; i < 100; i++) {
        str->append("x");
    }
    printf("  After 100 appends: capacity=%zu, length=%zu\n", 
           str->capacity(), str->length());
    
    /* Test shrink to fit */
    str->shrinkToFit();
    printf("  After shrinkToFit: capacity=%zu, length=%zu\n", 
           str->capacity(), str->length());
    
    /* Test reserve */
    str->reserve(1000);
    printf("  After reserve(1000): capacity=%zu\n", str->capacity());
    
    str->free();
    
    /* Test large string handling */
    String* large = StringMake("");
    for (i = 0; i < 1000; i++) {
        large->appendFormat("Line %d: This is a test of large string handling.\n", i);
    }
    printf("  Large string: length=%zu, capacity=%zu\n", 
           large->length(), large->capacity());
    printf("  Memory efficiency: %.1f%%\n", 
           (double)large->length() / large->capacity() * 100);
    
    large->free();
}

int main(void) {
    printf("=====================================\n");
    printf("   String Performance Testing\n");
    printf("=====================================\n\n");
    
    test_append_performance();
    test_search_performance();
    test_transformation_performance();
    test_split_join_performance();
    test_memory_efficiency();
    
    printf("\n=====================================\n");
    printf("✓ Performance testing completed!\n");
    printf("=====================================\n");
    
    return 0;
}