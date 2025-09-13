# Modular Trampoline Library

This is a modular version of the trampoline library that separates the core functionality from optional "class" implementations.

## Directory Structure

```
trampolines/
├── include/
│   └── trampolines/        # Headers for optional classes
│       └── string.h         # String class header
├── src/
│   └── classes/            # Source for optional classes
│       └── string.c        # String class implementation
├── lib/                    # Built libraries (created during build)
│   ├── libtrampoline.a/.dylib/.so
│   └── libtrampoline_string.a/.dylib/.so
├── trampoline.h            # Core library header
├── trampoline_arm64.c      # ARM64 implementation
├── trampoline_x86_64.c     # x86-64 implementation
├── trampoline_helpers.c    # Helper functions
└── Makefile.modular        # Build system
```

## Building

### Core Library Only

```bash
make -f Makefile.modular core
```

This builds:
- `lib/libtrampoline.a` - Static library
- `lib/libtrampoline.dylib` (macOS) or `lib/libtrampoline.so` (Linux) - Shared library

### Optional String Class

```bash
make -f Makefile.modular string
```

This builds:
- `lib/libtrampoline_string.a` - Static library
- `lib/libtrampoline_string.dylib` (macOS) or `lib/libtrampoline_string.so` (Linux) - Shared library

## Installation

### Install Core Library

```bash
sudo make -f Makefile.modular install-core
```

This installs:
- `/usr/local/include/trampoline.h`
- `/usr/local/lib/libtrampoline.a`
- `/usr/local/lib/libtrampoline.dylib` (or `.so`)

### Install String Class

```bash
sudo make -f Makefile.modular install-string
```

This installs:
- `/usr/local/include/trampolines/string.h`
- `/usr/local/lib/libtrampoline_string.a`
- `/usr/local/lib/libtrampoline_string.dylib` (or `.so`)

### Install Everything

```bash
sudo make -f Makefile.modular install-all
```

## Usage

### Using Core Library Only

```c
#include <trampoline.h>

// Your code using core trampoline functionality
```

Compile with:
```bash
gcc your_program.c -ltrampoline
```

### Using String Class

```c
#include <trampolines/string.h>

int main() {
    String* s = StringMake("Hello, World!");
    printf("%s\n", s->cStr());
    s->free();
    return 0;
}
```

Compile with:
```bash
gcc your_program.c -ltrampoline -ltrampoline_string
```

Note: You need both `-ltrampoline` and `-ltrampoline_string` because the string class depends on the core library.

## Adding New Classes

To add a new class (e.g., `array`):

1. Create the header: `include/trampolines/array.h`
2. Create the implementation: `src/classes/array.c`
3. Add build rules to `Makefile.modular`:

```makefile
# Array class variables
ARRAY_SRC = $(CLASSES_DIR)/array.c
ARRAY_OBJ = $(CLASSES_DIR)/array.o
ARRAY_LIB_STATIC = $(LIB_DIR)/libtrampoline_array.a
ARRAY_LIB_SHARED = $(LIB_DIR)/libtrampoline_array.$(DYLIB_EXT)

# Build target
array: $(ARRAY_LIB_STATIC) $(ARRAY_LIB_SHARED)

$(ARRAY_LIB_STATIC): $(ARRAY_OBJ) | $(LIB_DIR)
	$(AR) rcs $@ $^

$(ARRAY_LIB_SHARED): $(ARRAY_OBJ) | $(LIB_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(ARRAY_OBJ): $(ARRAY_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@
```

4. Add install/uninstall targets similar to the string class

## Benefits of This Structure

1. **Modularity**: Users only link what they need
2. **Clean separation**: Core library is independent
3. **Easy distribution**: Each class can be packaged separately
4. **Version independence**: Classes can be updated independently
5. **Minimal dependencies**: Classes only depend on core, not on each other

## Testing

Build and run the string test:
```bash
make -f Makefile.modular example-string
./examples/bin/string_test
```

## Uninstallation

```bash
# Remove everything
sudo make -f Makefile.modular uninstall-all

# Remove only string class
sudo make -f Makefile.modular uninstall-string

# Remove only core
sudo make -f Makefile.modular uninstall-core
```