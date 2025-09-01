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