/**
 * @file trampoline.c
 * @brief Umbrella file for platform-specific trampoline implementations.
 *
 * This file uses preprocessor directives to detect the target operating system
 * and architecture, and then includes the appropriate implementation file.
 * This is the only file that needs to be compiled to link the trampoline
 * functionality into an application.
 */

#include "trampoline.h"

// Check for Windows (both 32 and 64-bit)
#if defined(_WIN32) || defined(_WIN64)
    #if defined(_WIN64)
        #include "trampoline_x86_64_win.c"
    #else
        #include "trampoline_x86_win.c"
    #endif

// Check for POSIX-like systems (macOS, Linux, etc.)
#elif defined(__unix__) || defined(__APPLE__)
    #if defined(__aarch64__) // Apple Silicon, Linux ARM64
        #include "trampoline_arm64.c"
    #elif defined(__x86_64__) // Intel/AMD 64-bit
        #include "trampoline_x86_64.c"
    #elif defined(__arm__) // 32-bit ARM (Raspberry Pi, etc.)
        #include "trampoline_arm32.c"
    #elif defined(__i386__) // 32-bit x86
        #include "trampoline_x86.c"
    #else
        #error "Unsupported architecture for this trampoline implementation."
    #endif

#else
    #error "Unsupported operating system for this trampoline implementation."
#endif
