/**
 * @file string.h
 * @brief String structure with comprehensive string manipulation using trampolines
 *
 * This file provides a String object that encapsulates comprehensive string
 * functionality using the trampoline pattern. It supports common string operations,
 * manipulation, searching, transformations, and utilities with a clean, zero-cognitive-load API.
 *
 * @author Trampoline String Example
 * @date 2025
 */

#ifndef STRING_H
#define STRING_H

#include "../../trampoline.h"
#include <stddef.h>

/* C89 compatibility for bool */
#ifndef __cplusplus
  #ifndef __STDC_VERSION__
    typedef int bool;
    #define true 1
    #define false 0
  #elif __STDC_VERSION__ < 199901L
    typedef int bool;
    #define true 1
    #define false 0
  #else
    #include <stdbool.h>
  #endif
#endif

/**
 * @struct String
 * @brief String object with trampoline member functions for easy string manipulation
 *
 * The String struct provides an object-oriented interface for string handling in C.
 * All methods are implemented as trampoline functions that maintain context through
 * the self pointer, eliminating the need to pass the string instance explicitly.
 *
 * Key Features:
 * - Zero-cognitive-load API - methods operate on implicit self
 * - Automatic memory management - handles resizing internally
 * - Rich functionality - comprehensive string operations
 * - Immutable-friendly - many methods return new strings
 * - Safe operations - bounds checking and null safety
 *
 * @example Basic string operations
 * @code
 * String* str = StringMake("Hello");
 * str->append(" World");
 * printf("String: %s\n", str->cStr());
 * printf("Length: %zu\n", str->length());
 * printf("Contains 'World': %s\n", str->contains("World") ? "yes" : "no");
 * str->free();
 * @endcode
 *
 * @example String transformations
 * @code
 * String* name = StringMake("  john doe  ");
 * String* trimmed = name->trim();
 * String* upper = trimmed->toUpperCase();
 * printf("Formatted: %s\n", upper->cStr()); // "JOHN DOE"
 * 
 * name->free();
 * trimmed->free();
 * upper->free();
 * @endcode
 *
 * @example String building and formatting
 * @code
 * String* builder = StringMake("");
 * builder->appendFormat("User: %s, ", username);
 * builder->appendFormat("ID: %d, ", userId);
 * builder->appendFormat("Score: %.2f", score);
 * 
 * String* result = builder->toString();
 * printf("%s\n", result->cStr());
 * 
 * builder->free();
 * result->free();
 * @endcode
 */
typedef struct String {
    /* ================================================================ */
    /* Core String Access                                              */
    /* ================================================================ */
    
    /**
     * @brief Get the C string pointer (null-terminated)
     * @return Const pointer to internal string buffer
     * @note String is always null-terminated for C compatibility
     */
    const char* (*cStr)(void);
    
    /**
     * @brief Get the length of the string (excluding null terminator)
     * @return Number of characters in the string
     */
    size_t (*length)(void);
    
    /**
     * @brief Get the capacity of the internal buffer
     * @return Current allocated size including null terminator
     */
    size_t (*capacity)(void);
    
    /**
     * @brief Check if the string is empty
     * @return true if length is 0, false otherwise
     */
    bool (*isEmpty)(void);
    
    /**
     * @brief Get character at specific index
     * @param index Position in string (0-based)
     * @return Character at index, or '\0' if out of bounds
     */
    char (*charAt)(size_t index);
    
    /* ================================================================ */
    /* String Modification (In-Place)                                  */
    /* ================================================================ */
    
    /**
     * @brief Append a string to this string
     * @param str String to append (null-safe)
     * @return true if successful, false on error
     */
    bool (*append)(const char* str);
    
    /**
     * @brief Append a single character
     * @param ch Character to append
     * @return true if successful, false on error
     */
    bool (*appendChar)(char ch);
    
    /**
     * @brief Append formatted string (like sprintf)
     * @param format Printf-style format string
     * @param ... Variable arguments
     * @return true if successful, false on error
     */
    bool (*appendFormat)(const char* format, ...);
    
    /**
     * @brief Prepend a string to the beginning
     * @param str String to prepend (null-safe)
     * @return true if successful, false on error
     */
    bool (*prepend)(const char* str);
    
    /**
     * @brief Insert string at specific position
     * @param index Position to insert at (0 = beginning)
     * @param str String to insert
     * @return true if successful, false on error or invalid index
     */
    bool (*insert)(size_t index, const char* str);
    
    /**
     * @brief Replace all occurrences of a substring
     * @param find String to search for
     * @param replace String to replace with
     * @return Number of replacements made
     */
    size_t (*replace)(const char* find, const char* replace);
    
    /**
     * @brief Replace first occurrence of a substring
     * @param find String to search for
     * @param replace String to replace with
     * @return true if replacement made, false if not found
     */
    bool (*replaceFirst)(const char* find, const char* replace);
    
    /**
     * @brief Clear the string (make it empty)
     */
    void (*clear)(void);
    
    /**
     * @brief Set this string to a new value
     * @param str New string value (will be copied)
     * @return true if successful, false on error
     */
    bool (*set)(const char* str);
    
    /**
     * @brief Reverse the string in place
     */
    void (*reverse)(void);
    
    /**
     * @brief Convert to uppercase in place
     */
    void (*toUpperCaseInPlace)(void);
    
    /**
     * @brief Convert to lowercase in place
     */
    void (*toLowerCaseInPlace)(void);
    
    /* ================================================================ */
    /* String Creation (Returns New String)                            */
    /* ================================================================ */
    
    /**
     * @brief Create a substring from start index to end
     * @param start Starting index (inclusive)
     * @param length Number of characters (0 = to end)
     * @return New String object or NULL on error
     */
    struct String* (*substring)(size_t start, size_t length);
    
    /**
     * @brief Create a new string with whitespace removed from both ends
     * @return New trimmed String object
     */
    struct String* (*trim)(void);
    
    /**
     * @brief Create a new string with whitespace removed from left
     * @return New trimmed String object
     */
    struct String* (*trimLeft)(void);
    
    /**
     * @brief Create a new string with whitespace removed from right
     * @return New trimmed String object
     */
    struct String* (*trimRight)(void);
    
    /**
     * @brief Create uppercase copy of this string
     * @return New uppercase String object
     */
    struct String* (*toUpperCase)(void);
    
    /**
     * @brief Create lowercase copy of this string
     * @return New lowercase String object
     */
    struct String* (*toLowerCase)(void);
    
    /**
     * @brief Create a copy of this string
     * @return New String object with same content
     */
    struct String* (*clone)(void);
    
    /**
     * @brief Repeat this string n times
     * @param count Number of repetitions
     * @return New String with repeated content
     */
    struct String* (*repeat)(size_t count);
    
    /* ================================================================ */
    /* String Searching                                                */
    /* ================================================================ */
    
    /**
     * @brief Check if string contains a substring
     * @param needle Substring to search for
     * @return true if found, false otherwise
     */
    bool (*contains)(const char* needle);
    
    /**
     * @brief Check if string starts with a prefix
     * @param prefix String to check at beginning
     * @return true if string starts with prefix
     */
    bool (*startsWith)(const char* prefix);
    
    /**
     * @brief Check if string ends with a suffix
     * @param suffix String to check at end
     * @return true if string ends with suffix
     */
    bool (*endsWith)(const char* suffix);
    
    /**
     * @brief Find first occurrence of substring
     * @param needle String to search for
     * @return Index of first occurrence, or (size_t)-1 if not found
     */
    size_t (*indexOf)(const char* needle);
    
    /**
     * @brief Find last occurrence of substring
     * @param needle String to search for
     * @return Index of last occurrence, or (size_t)-1 if not found
     */
    size_t (*lastIndexOf)(const char* needle);
    
    /**
     * @brief Find first occurrence of any character in set
     * @param chars Set of characters to search for
     * @return Index of first match, or (size_t)-1 if not found
     */
    size_t (*indexOfAny)(const char* chars);
    
    /**
     * @brief Count occurrences of substring
     * @param needle Substring to count
     * @return Number of non-overlapping occurrences
     */
    size_t (*count)(const char* needle);
    
    /* ================================================================ */
    /* String Splitting and Joining                                    */
    /* ================================================================ */
    
    /**
     * @brief Split string by delimiter
     * @param delimiter String to split on
     * @param out_count Pointer to store number of parts
     * @return Array of new String objects (caller must free array and strings)
     */
    struct String** (*split)(const char* delimiter, size_t* out_count);
    
    /**
     * @brief Split string by any character in set
     * @param chars Set of delimiter characters
     * @param out_count Pointer to store number of parts
     * @return Array of new String objects (caller must free array and strings)
     */
    struct String** (*splitAny)(const char* chars, size_t* out_count);
    
    /**
     * @brief Split string into lines
     * @param out_count Pointer to store number of lines
     * @return Array of new String objects (caller must free array and strings)
     */
    struct String** (*splitLines)(size_t* out_count);
    
    /**
     * @brief Join array of strings with this string as separator
     * @param strings Array of String objects to join
     * @param count Number of strings in array
     * @return New joined String object
     */
    struct String* (*join)(struct String** strings, size_t count);
    
    /* ================================================================ */
    /* String Comparison                                               */
    /* ================================================================ */
    
    /**
     * @brief Compare with another string (case-sensitive)
     * @param other String to compare with
     * @return 0 if equal, <0 if this<other, >0 if this>other
     */
    int (*compare)(const char* other);
    
    /**
     * @brief Compare with another string (case-insensitive)
     * @param other String to compare with
     * @return 0 if equal, <0 if this<other, >0 if this>other
     */
    int (*compareIgnoreCase)(const char* other);
    
    /**
     * @brief Check if equal to another string
     * @param other String to compare with
     * @return true if strings are equal
     */
    bool (*equals)(const char* other);
    
    /**
     * @brief Check if equal to another string (case-insensitive)
     * @param other String to compare with
     * @return true if strings are equal ignoring case
     */
    bool (*equalsIgnoreCase)(const char* other);
    
    /* ================================================================ */
    /* String Utilities                                                */
    /* ================================================================ */
    
    /**
     * @brief Check if string represents a valid integer
     * @return true if string can be parsed as integer
     */
    bool (*isInteger)(void);
    
    /**
     * @brief Check if string represents a valid float
     * @return true if string can be parsed as float
     */
    bool (*isFloat)(void);
    
    /**
     * @brief Check if string contains only alphabetic characters
     * @return true if all characters are letters
     */
    bool (*isAlpha)(void);
    
    /**
     * @brief Check if string contains only digits
     * @return true if all characters are digits
     */
    bool (*isDigit)(void);
    
    /**
     * @brief Check if string contains only alphanumeric characters
     * @return true if all characters are letters or digits
     */
    bool (*isAlphaNumeric)(void);
    
    /**
     * @brief Check if string contains only whitespace
     * @return true if all characters are whitespace
     */
    bool (*isWhitespace)(void);
    
    /**
     * @brief Convert string to integer
     * @param default_value Value to return if conversion fails
     * @return Parsed integer or default_value
     */
    int (*toInt)(int default_value);
    
    /**
     * @brief Convert string to float
     * @param default_value Value to return if conversion fails
     * @return Parsed float or default_value
     */
    float (*toFloat)(float default_value);
    
    /**
     * @brief Convert string to double
     * @param default_value Value to return if conversion fails
     * @return Parsed double or default_value
     */
    double (*toDouble)(double default_value);
    
    /**
     * @brief Calculate hash code for the string
     * @return Hash value suitable for hash tables
     */
    size_t (*hash)(void);
    
    /**
     * @brief Get a read-only version of the string
     * @return New String that cannot be modified
     * @note Not enforced by compiler, but convention
     */
    struct String* (*toString)(void);
    
    /* ================================================================ */
    /* Memory Management                                               */
    /* ================================================================ */
    
    /**
     * @brief Reserve capacity for string growth
     * @param new_capacity Minimum capacity to ensure
     * @return true if successful, false on allocation failure
     */
    bool (*reserve)(size_t new_capacity);
    
    /**
     * @brief Shrink capacity to fit current string length
     * @return true if successful, false on error
     */
    bool (*shrinkToFit)(void);
    
    /**
     * @brief Free the String and all resources
     * @warning Do not use the object after calling this
     */
    void (*free)(void);
} String;

/* ======================================================================== */
/* String Creation Functions                                                */
/* ======================================================================== */

/**
 * @brief Create a new String from a C string
 * @param str Initial string value (can be NULL for empty string)
 * @return New String object or NULL on allocation failure
 */
String* StringMake(const char* str);

/**
 * @brief Create a new String with specific initial capacity
 * @param str Initial string value (can be NULL)
 * @param capacity Initial buffer capacity
 * @return New String object or NULL on allocation failure
 */
String* StringMakeWithCapacity(const char* str, size_t capacity);

/**
 * @brief Create a new String from formatted input
 * @param format Printf-style format string
 * @param ... Variable arguments
 * @return New String object or NULL on allocation failure
 */
String* StringMakeFormat(const char* format, ...);

/**
 * @brief Create a String from integer
 * @param value Integer value to convert
 * @return New String object or NULL on allocation failure
 */
String* StringFromInt(int value);

/**
 * @brief Create a String from float
 * @param value Float value to convert
 * @param precision Number of decimal places
 * @return New String object or NULL on allocation failure
 */
String* StringFromFloat(float value, int precision);

/**
 * @brief Create a String from double
 * @param value Double value to convert
 * @param precision Number of decimal places
 * @return New String object or NULL on allocation failure
 */
String* StringFromDouble(double value, int precision);

/* ======================================================================== */
/* String Array Utilities                                                   */
/* ======================================================================== */

/**
 * @brief Free an array of String objects
 * @param strings Array of String pointers
 * @param count Number of strings in array
 */
void StringArray_Free(String** strings, size_t count);

/**
 * @brief Join array of C strings into a String
 * @param strings Array of C string pointers
 * @param count Number of strings
 * @param separator Separator between strings
 * @return New String object with joined content
 */
String* StringArray_Join(const char** strings, size_t count, const char* separator);

#endif /* STRING_H */
