# Claude Session Notes - Trampolines Project

## Project Overview
This project implements "trampolines" - dynamically generated function wrappers that automatically inject a context pointer as the first parameter, allowing C structs to have method-like function pointers that don't require explicit `self` parameters.

## Key Fixes Applied

### 1. ARM64 (trampoline_arm64.c)
**Issues Fixed:**
- `mov_imm64` function was using undefined `code` variable instead of parameter `b`
- Register shifting was incorrect (was `mov x{i+1}, x{i}`, fixed to `mov x{i}, x{i-1}`)
- Stack manipulation immediates were incorrectly scaled

**Status:** ✅ Working and tested on macOS

### 2. i386/x86 (trampoline_x86.c)
**Issues Fixed:**
- Stack manipulation was completely wrong, causing segfaults
- Simplified to: pop return address → push context → push return address → jump
- Now handles both zero-argument and multi-argument cases correctly

**Status:** ⚠️ Fixed but untested (no i386 hardware available)

### 3. ARM32 (trampoline_arm32.c)
**Issues Fixed:**
- Unsafe use of PC register (r15) for padding - now uses proper zero value
- PC-relative offset calculations were hardcoded and brittle
- Now dynamically calculates offsets to literal pool

**Status:** ⚠️ Fixed but untested (no ARM32 hardware available)

### 4. Windows i386 (trampoline_x86_win.c)
**Issues Fixed:**
- Original ignored `public_argc` completely
- Now mirrors the Unix i386 fix with Windows-specific memory APIs

**Status:** ⚠️ Fixed but untested (no Windows environment)

### 5. Windows x86_64 (trampoline_x86_64_win.c)
**Issues Fixed:**
- Complete rewrite - original was overly complex and broken
- Now properly handles Windows x64 ABI (RCX, RDX, R8, R9 + stack)
- Correctly shifts all arguments and handles 5+ argument cases

**Status:** ⚠️ Fixed but untested (no Windows environment)

## Architecture-Specific Calling Conventions

### x86 (i386) - cdecl
- All arguments on stack
- Stack layout: `[return_addr][arg0][arg1]...`
- Caller cleans up stack

### x86_64 Unix (System V ABI)
- First 6 args in: RDI, RSI, RDX, RCX, R8, R9
- Additional args on stack
- Red zone: 128 bytes below RSP

### x86_64 Windows
- First 4 args in: RCX, RDX, R8, R9
- Shadow space required: 32 bytes
- Stack args start at [RSP+0x28]

### ARM64 (AAPCS64)
- First 8 args in: X0-X7
- Additional args on stack
- Stack must be 16-byte aligned

### ARM32 (AAPCS)
- First 4 args in: R0-R3
- Additional args on stack
- Stack must be 8-byte aligned

## Common Pattern
All implementations follow the same basic pattern:
1. Save/shift existing arguments to make room
2. Insert context as the first argument
3. Jump (not call) to the target function
4. Target function returns directly to original caller

## Testing Status
- ✅ **Tested Working:** ARM64 (macOS), example programs run correctly
- ⚠️ **Fixed but Untested:** i386, ARM32, Windows i386, Windows x86_64
- ✅ **Not Modified:** x86_64 Unix, PowerPC variants (already working)

## Helper Infrastructure
- `trampoline_helpers.c`: Provides allocation tracking and validation
- Macro helpers in `trampoline.h`: TRAMP_GETTER, TRAMP_SETTER, etc.
- `array_example.c`: Demonstrates practical usage

## Next Steps for Future Sessions
1. Test i386 implementation on appropriate hardware
2. Test ARM32 on Raspberry Pi or similar
3. Test Windows implementations on Windows
4. Consider adding more helper macros as outlined in ABOUT.md
5. Potentially add support for more architectures (RISC-V, MIPS, etc.)

## Key Debugging Insights
- Always verify stack alignment requirements for each architecture
- PC-relative addressing in ARM needs careful offset calculation
- Register shifting must be done in correct order to avoid clobbering
- Windows and Unix have significantly different calling conventions even on same CPU
- Simple, clear implementations are better than complex "clever" ones

## Files Modified in Previous Sessions
- `trampoline_arm64.c` - Fixed and working
- `trampoline_x86.c` - Fixed, needs testing  
- `trampoline_arm32.c` - Fixed, needs testing
- `trampoline_x86_win.c` - Rewritten, needs testing
- `trampoline_x86_64_win.c` - Rewritten, needs testing

## Network Example Implementation (Current Session)

### Overview
Created a complete HTTP client implementation using trampolines to demonstrate
practical usage of the pattern for network programming. The implementation uses
POSIX sockets for portability between macOS and Linux without external dependencies.

### Files Created
1. **examples/network/network_request.h** - Public interface with doxygen docs
2. **examples/network/network_request_impl.c** - Request implementation
3. **examples/network/network_response.h** - Response interface with doxygen docs  
4. **examples/network/network_response_impl.c** - Response implementation
5. **examples/network/network_example.c** - Example usage program
6. **examples/network/Makefile** - Build system using umbrella trampoline.c

### Key Features Implemented
- **NetworkRequest struct**: HTTP request builder with trampoline methods
  - URL parsing and validation
  - HTTP method support (GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS)
  - Header management with linked list storage
  - Request body support with automatic Content-Length
  - Configurable port and timeout
  - `send()` method that executes the request

- **NetworkResponse struct**: HTTP response parser with trampoline methods
  - Status code and message parsing
  - Header parsing and case-insensitive lookup
  - Body content handling
  - Error state management
  - `isSuccess()` helper for 2xx status codes

- **Socket Implementation**:
  - DNS resolution via `gethostbyname()`
  - TCP socket creation and connection
  - HTTP/1.1 protocol implementation
  - Timeout support using socket options
  - Proper resource cleanup on errors

### Macro Additions to trampoline.h
Added new macros for private struct member access:
- `TRAMP_SETTER_` - Setter for private struct members
- `TRAMP_STRING_SETTER_` - String setter with memory management for private members

### Code Quality Improvements
- Comprehensive doxygen documentation with examples
- All lines kept under 80 columns for readability
- Consistent error handling throughout
- Memory leak prevention with proper cleanup

### Build System
Created Makefile that:
- Uses umbrella `trampoline.c` for automatic platform detection
- Provides targets: all, run, clean, debug, docs, help
- Successfully tested on macOS ARM64

### Testing Status
✅ **Fully Working**: Network example successfully makes HTTP requests to httpbin.org
- GET requests with headers work correctly
- POST requests with JSON bodies work correctly  
- Response parsing handles status, headers, and body
- All trampoline functions operate as expected

### Limitations Noted
- HTTP only (HTTPS would require SSL/TLS library)
- Fixed 64KB response buffer size
- Basic HTTP/1.1 implementation
- Synchronous/blocking operations only

### Architecture Benefits Demonstrated
The network example shows how trampolines enable:
1. Object-oriented design in C with method-like syntax
2. Clean separation of public interface and private implementation
3. Encapsulation of complex state (sockets, headers, buffers)
4. Intuitive API: `request->setHeader("User-Agent", "MyApp")`
5. Resource management: `response->free()` cleans up everything

## Map v2 Example Implementation (Current Session Continued)

### Overview
Debugged and fixed critical segfault issues in the Map v2 with MapNode integration example,
then converted all examples to C89 compatibility for m68k platform support, and enhanced 
the build system to support all sample applications.

### Critical Segfault Fix
**Root Cause**: Incorrect struct casting between `Map` and `MapPrivate` structures. The original
implementation cast `MapPrivate*` directly to `Map*`, but the trampoline self-pointer expected
`Map*` while the actual memory layout started with `MapPrivate` fields.

**Solution Applied**: Implemented embedded struct pattern:
```c
typedef struct MapPrivate {
    Map public;          // Public interface MUST be first
    MapEntry** buckets;  // Private implementation fields
    size_t capacity;
    size_t size;
    float max_load_factor;
} MapPrivate;

// Usage: Map* map = &priv->public; instead of (Map*)priv
```

**Result**: Both `map_test` and `simple_map_test` now run without segfaults, demonstrating
comprehensive Map v2 functionality with zero-cognitive-load API.

### C89 Compatibility Conversion

**Rationale**: Support for m68k variants which use C89 compilers.

**Files Updated for C89 Compliance**:
- `examples/network/network_example.c`
- `examples/network/network_request_impl.c` 
- `examples/network/network_response_impl.c`
- `examples/map/map_example.c`
- `examples/map/map_impl.c`
- `examples/map/mapnode_impl.c`
- `examples/map/debug_map.c`

**Key Changes Made**:
1. **Variable Declarations**: Moved all variable declarations to beginning of blocks
2. **For Loop Variables**: Changed `for (size_t i = 0; ...)` to:
   ```c
   size_t i;
   for (i = 0; i < count; i++) { ... }
   ```
3. **Block Scoping**: Added proper `{ }` blocks where needed for variable scope
4. **Include Paths**: Fixed relative paths from `<trampoline.h>` to `"../../trampoline.h"`

**Verification**: All examples compile with `-std=c89` flag and maintain full functionality.

### Enhanced Build System (Map Example)

**Makefile Improvements**:
- **All Sample Apps**: Now builds 6 sample applications instead of just 1:
  - `map_test` - Complete Map v2 demonstration  
  - `mapnode_test` - MapNode-only tests
  - `usage_example` - Usage demonstrations
  - `simple_map_test` - Basic functionality test
  - `debug_map` - Debug tracing example
  - `minimal_map` - Minimal implementation test

- **Enhanced Clean Target**: Removes all binaries, .dSYM directories, and documentation
- **New Test Targets**: `test-simple`, `test-debug`, `test-minimal`, `test-all`
- **Updated Documentation**: Help target shows all available commands

**Dependencies Fixed**: 
- Ensured all sample apps include necessary implementation files
- Fixed include paths for consistency
- Added proper source file dependencies in Makefile rules

### Network Example Fix
**Issue**: Network example was broken due to incorrect brace pairing from C89 conversion
**Resolution**: Fixed control flow structure and variable declarations in POST request section
**Result**: Network example builds and runs correctly, making successful HTTP GET/POST requests

### Testing Status Summary
✅ **All Examples Working**:
- Network example: Successfully makes HTTP requests  
- Map example: All 6 sample apps compile and run correctly
- Segfault issues resolved in both test executables
- C89 compatibility maintained across all examples
- Build systems enhanced and fully functional

### Files Modified This Session
- `examples/map/map_impl.c` - Fixed embedded struct pattern, C89 compliance
- `examples/map/map_example.c` - C89 variable declaration fixes
- `examples/map/mapnode_impl.c` - C89 for-loop fixes  
- `examples/map/debug_map.c` - Added map implementation include, fixed include path
- `examples/map/simple_map_test.c` - Already fixed in previous session continuation
- `examples/map/Makefile` - Complete enhancement for all sample apps
- `examples/network/network_example.c` - Fixed brace structure from C89 conversion
- `examples/network/network_request_impl.c` - C89 compliance fixes
- `examples/network/network_response_impl.c` - C89 compliance fixes

### Key Architectural Insights
1. **Embedded Struct Pattern**: Critical for trampoline self-pointer correctness
2. **Memory Layout**: Public interface must be first member for casting to work
3. **C89 Compatibility**: Essential for legacy platform support (m68k variants)
4. **Build System Design**: Comprehensive sample app coverage improves usability

### Current Project Status
- ✅ **Core Trampolines**: ARM64 working, others architectures fixed but need testing
- ✅ **Network Example**: Fully functional HTTP client with C89 compatibility
- ✅ **Map Example**: Complete hash table with MapNode integration, all samples working
- ✅ **Build Systems**: Enhanced Makefiles with comprehensive targets and cleanup
- ✅ **Platform Compatibility**: C89 compliance for widest possible platform support