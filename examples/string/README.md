# String Trampoline Example

A comprehensive String struct implementation with trampoline member functions, providing an easy-to-read, easy-to-use, class-like capability for string manipulation in C.

## Features

The String struct provides a zero-cognitive-load API with over 50 member functions covering:

### Core Operations
- `cStr()` - Get C string pointer
- `length()` - Get string length
- `capacity()` - Get buffer capacity
- `isEmpty()` - Check if empty
- `charAt()` - Get character at index

### String Modification (In-Place)
- `append()` - Append string
- `appendChar()` - Append character
- `appendFormat()` - Printf-style append
- `prepend()` - Prepend string
- `insert()` - Insert at position
- `replace()` - Replace all occurrences
- `replaceFirst()` - Replace first occurrence
- `clear()` - Clear string
- `set()` - Set new value
- `reverse()` - Reverse in place
- `toUpperCaseInPlace()` - Convert to uppercase
- `toLowerCaseInPlace()` - Convert to lowercase

### String Creation (Returns New String)
- `substring()` - Extract substring
- `trim()` - Remove whitespace from both ends
- `trimLeft()` - Remove left whitespace
- `trimRight()` - Remove right whitespace
- `toUpperCase()` - Create uppercase copy
- `toLowerCase()` - Create lowercase copy
- `clone()` - Create copy
- `repeat()` - Repeat string n times

### String Searching
- `contains()` - Check for substring
- `startsWith()` - Check prefix
- `endsWith()` - Check suffix
- `indexOf()` - Find first occurrence
- `lastIndexOf()` - Find last occurrence
- `indexOfAny()` - Find any character
- `count()` - Count occurrences

### String Splitting & Joining
- `split()` - Split by delimiter
- `splitAny()` - Split by any character
- `splitLines()` - Split into lines
- `join()` - Join array of strings

### String Comparison
- `compare()` - Case-sensitive compare
- `compareIgnoreCase()` - Case-insensitive compare
- `equals()` - Check equality
- `equalsIgnoreCase()` - Case-insensitive equality

### String Utilities
- `isInteger()` - Check if valid integer
- `isFloat()` - Check if valid float
- `isAlpha()` - Check if alphabetic
- `isDigit()` - Check if digits only
- `isAlphaNumeric()` - Check if alphanumeric
- `isWhitespace()` - Check if whitespace
- `toInt()` - Convert to integer
- `toFloat()` - Convert to float
- `toDouble()` - Convert to double
- `hash()` - Calculate hash code

### Memory Management
- `reserve()` - Reserve capacity
- `shrinkToFit()` - Shrink to fit content
- `free()` - Free all resources

## Usage Example

```c
#include "string.h"
#include "string_impl.c"

int main(void) {
    /* Create a string - simple and intuitive */
    String* name = StringMake("John Doe");
    
    /* Use member functions without passing 'self' */
    printf("Name: %s\n", name->cStr());
    printf("Length: %zu\n", name->length());
    
    /* Transform strings naturally */
    String* upper = name->toUpperCase();
    printf("Uppercase: %s\n", upper->cStr());
    
    /* Build strings fluently */
    String* greeting = StringMake("Hello, ");
    greeting->append(name->cStr());
    greeting->append("!");
    printf("%s\n", greeting->cStr());
    
    /* Search operations are intuitive */
    if (greeting->contains("John")) {
        printf("Found John!\n");
    }
    
    /* Split strings easily */
    String* email = StringMake("user@example.com");
    size_t count;
    String** parts = email->split("@", &count);
    if (count == 2) {
        printf("User: %s\n", parts[0]->cStr());
        printf("Domain: %s\n", parts[1]->cStr());
    }
    
    /* Clean up */
    StringArray_Free(parts, count);
    name->free();
    upper->free();
    greeting->free();
    email->free();
    
    return 0;
}
```

## Building

To build the examples:

```bash
# Build the demo program
make string_demo

# Run the demo
./string_demo

# Build all examples (requires fixing C89 issues in other files)
make all

# Clean build artifacts
make clean
```

## Implementation Details

The String implementation uses:
- **Trampoline pattern** - Member functions receive implicit `self` pointer
- **Dynamic memory** - Automatic buffer growth as needed
- **Efficient operations** - Optimized for common use cases
- **Safe defaults** - Null-safe operations throughout
- **C89 compatible** - Works with older compilers (with minor adjustments)

## Key Benefits

1. **Zero Cognitive Load** - No need to remember to pass string as first parameter
2. **Natural API** - Methods feel like object-oriented programming
3. **Comprehensive** - Over 50 methods cover all common string operations
4. **Memory Safe** - Automatic memory management with clear ownership
5. **Performance** - Efficient implementation with capacity management
6. **Readable** - Code using String is self-documenting

## Files

- `string.h` - Public interface with comprehensive documentation
- `string_impl.c` - Implementation of all string functions
- `string_demo.c` - Simple C89-compliant demonstration
- `string_example.c` - Comprehensive test suite
- `simple_string_test.c` - Basic usage example
- `string_performance.c` - Performance benchmarks

## Notes

This implementation demonstrates how the trampoline pattern can create a powerful, easy-to-use string library in C that rivals higher-level languages while maintaining C's efficiency and control.