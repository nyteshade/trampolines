# Trampoline Library Build Guide

## Project Structure

The project is organized to separate the core trampoline library from optional "class" libraries:

```
trampolines/
├── Makefile                 # Core library build
├── Makefile.config          # SSL/OpenSSL configuration
├── trampoline.h             # Core header
├── trampoline_arm64.c      # ARM64 implementation
├── trampoline_x86_64.c     # x86-64 implementation
├── trampoline_helpers.c    # Shared helpers
├── lib/                     # Built libraries go here
│   ├── libtrampoline.a     # Core static library
│   ├── libtrampoline.dylib # Core dynamic library
│   ├── libtrampolines.a    # Classes static library
│   └── libtrampolines.dylib # Classes dynamic library
└── trampolines/             # Optional classes
    ├── Makefile
    ├── include/trampolines/
    │   ├── string.h         # String class
    │   └── network.h        # Network classes
    └── src/classes/
        ├── string.c
        ├── network_common.c/h
        ├── network_request.c
        └── network_response.c
```

## Quick Start

### Build Everything
```bash
make all-with-classes        # Build core + classes
```

### Build Core Only
```bash
make                         # Build just libtrampoline
```

### Build Classes Only
```bash
make -C trampolines          # Build just libtrampolines
```

## Libraries

### Core Library (libtrampoline)
- **Purpose**: Core trampoline infrastructure
- **Headers**: `#include <trampoline.h>`
- **Link**: `-ltrampoline`
- **Required**: Always

### Classes Library (libtrampolines)
- **Purpose**: Optional utility classes (String, Network, etc.)
- **Headers**: `#include <trampolines/string.h>`, `#include <trampolines/network.h>`
- **Link**: `-ltrampoline -ltrampolines`
- **Required**: Only if using classes
- **Dependencies**: Requires libtrampoline

## Installation

### Install Everything
```bash
make install-all             # Install to /usr/local
```

### Install with Custom Prefix
```bash
make install-all INSTALL_PREFIX=/opt/local
```

### Install Core Only
```bash
make install                 # Just core library
```

## SSL/OpenSSL Configuration

The network classes support SSL/TLS through OpenSSL. Configuration is automatic but can be customized:

### View Current Configuration
```bash
make -f Makefile.config show
```

### Build with Custom OpenSSL
```bash
# MacPorts OpenSSL
make all-with-classes OPENSSL_PREFIX=/opt/local

# Homebrew OpenSSL
make all-with-classes OPENSSL_PREFIX=/opt/homebrew/opt/openssl

# System OpenSSL
make all-with-classes OPENSSL_PREFIX=/usr
```

### Build without SSL
```bash
make all-with-classes SSL_ENABLED=no
```

### Platform-Specific Notes

#### macOS Tiger/Leopard (PowerPC)
```bash
# Use MacPorts OpenSSL
make all-with-classes OPENSSL_PREFIX=/opt/local
```

#### Modern macOS (ARM64/x86_64)
```bash
# Usually auto-detected, but can override
make all-with-classes OPENSSL_PREFIX=/opt/homebrew/opt/openssl
```

#### Linux
```bash
# Usually uses system OpenSSL
make all-with-classes
```

## Universal Binary (macOS)

For building universal binaries, use the `build_macos.sh` script:

```bash
./build_macos.sh            # Build all architectures
./build_macos.sh install    # Build and install
```

This creates universal binaries with up to 5 architectures:
- PowerPC (32-bit)
- PowerPC (64-bit)
- Intel (32-bit)
- Intel (64-bit)  
- ARM (64-bit)

The script uses `lipo` to combine individual architecture builds.

## Usage Examples

### Core Library Only
```c
#include <trampoline.h>

// Your code using trampolines
```

Compile:
```bash
gcc myprogram.c -ltrampoline
```

### With String Class
```c
#include <trampoline.h>
#include <trampolines/string.h>

int main() {
    String* s = StringMake("Hello");
    s->append(" World");
    printf("%s\n", s->cStr());
    s->free();
    return 0;
}
```

Compile:
```bash
gcc myprogram.c -ltrampoline -ltrampolines
```

### With Network Classes (SSL)
```c
#include <trampoline.h>
#include <trampolines/network.h>
#include <trampolines/string.h>

int main() {
    NetworkRequest* req = NetworkRequestMake("https://api.example.com", HTTP_GET);
    NetworkResponse* resp = req->send();
    if (resp) {
        String* body = resp->body();
        printf("Response: %s\n", body->cStr());
        resp->free();
    }
    req->free();
    return 0;
}
```

Compile:
```bash
gcc myprogram.c -ltrampoline -ltrampolines -lssl -lcrypto
```

## Troubleshooting

### SSL Headers Not Found
```bash
# Check SSL configuration
make -f Makefile.config show

# Specify OpenSSL location
make all-with-classes OPENSSL_PREFIX=/path/to/openssl
```

### Library Not Found at Runtime
```bash
# Set library path (macOS)
export DYLD_LIBRARY_PATH=/usr/local/lib

# Set library path (Linux)
export LD_LIBRARY_PATH=/usr/local/lib

# Or use rpath when compiling
gcc myprogram.c -ltrampoline -ltrampolines -Wl,-rpath,/usr/local/lib
```

### Architecture Mismatch
```bash
# Check library architecture
lipo -info lib/libtrampoline.dylib

# Build for specific architecture
gcc -arch arm64 ...
```

## Clean Build
```bash
make clean                   # Clean core
make -C trampolines clean    # Clean classes
# or
make clean && make -C trampolines clean
```

## Development

### Adding New Classes

1. Add header to `trampolines/include/trampolines/`
2. Add source to `trampolines/src/classes/`
3. Update `trampolines/Makefile` to include new files
4. Use String class where appropriate for better API

### Testing Individual Classes
```bash
make -C trampolines string-only    # Build just String
make -C trampolines network-only   # Build just Network
```

## License

See LICENSE file in the project root.