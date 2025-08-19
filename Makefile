# Makefile for the Trampoline Project

# --- Configuration ---
CC = clang
CFLAGS = -Wall -Wextra -std=c99 -g
# Flags for building Position-Independent Code for shared libraries
SHARED_CFLAGS = $(CFLAGS) -fPIC

# Detect the operating system
OS = $(shell uname)

# Set library extensions based on OS
ifeq ($(OS), Darwin)
	SHARED_LIB_EXT = .dylib
else
	SHARED_LIB_EXT = .so
endif

# --- File Definitions ---
SRCS = main.c trampoline.c
SHARED_LIB_SRC = trampoline.c
STATIC_LIB_SRC = trampoline.c

# --- Target Names ---
SHARED_LIB = libtrampoline$(SHARED_LIB_EXT)
STATIC_LIB = libtrampoline.a

# --- Primary Targets ---

# The default target builds a universal binary on macOS or a native binary on Linux.
all:
ifeq ($(OS), Darwin)
	$(MAKE) universal
else
	$(MAKE) trampoline_demo_native
endif

# Help target to display available commands
help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Available targets:"
	@echo "  all                  Builds the default executable for the platform (universal on macOS)."
	@echo "  universal            (macOS only) Builds a universal executable (x86_64 and arm64)."
	@echo "  shared-lib           Builds a shared library (.dylib or .so)."
	@echo "  static-lib           Builds a static library (.a)."
	@echo "  clean                Removes all build artifacts."
	@echo "  help                 Shows this help message."
	@echo ""

# --- Library Targets ---

# Generic target to build the shared library for the current platform.
shared-lib: $(SHARED_LIB)

# Generic target to build the static library for the current platform.
static-lib: $(STATIC_LIB)

# --- Platform-Specific Library Implementations ---

# macOS (Darwin) specific library builds
ifeq ($(OS), Darwin)

# Create a universal shared library
$(SHARED_LIB): libtrampoline.x86_64.dylib libtrampoline.arm64.dylib
	@echo "Creating universal shared library..."
	lipo -create -output $@ $^

# Create a universal static library
$(STATIC_LIB): libtrampoline.x86_64.a libtrampoline.arm64.a
	@echo "Creating universal static library..."
	lipo -create -output $@ $^

# Rule to build arch-specific shared libraries
libtrampoline.x86_64.dylib: trampoline.x86_64.shared.o
	$(CC) --target=x86_64-apple-darwin -dynamiclib -o $@ $^
libtrampoline.arm64.dylib: trampoline.arm64.shared.o
	$(CC) --target=arm64-apple-darwin -dynamiclib -o $@ $^

# Rule to build arch-specific static libraries
libtrampoline.x86_64.a: trampoline.x86_64.o
	ar rcs $@ $^
libtrampoline.arm64.a: trampoline.arm64.o
	ar rcs $@ $^

# Linux specific library builds
else

# Create a native shared library (.so)
$(SHARED_LIB): $(SHARED_LIB_SRC:.c=.shared.o)
	@echo "Building Linux shared library..."
	$(CC) $(SHARED_CFLAGS) -shared -o $@ $^

# Create a native static library (.a)
$(STATIC_LIB): $(STATIC_LIB_SRC:.c=.o)
	@echo "Building Linux static library..."
	ar rcs $@ $^

endif

# --- Executable Targets ---

universal: trampoline_demo_x86_64 trampoline_demo_arm64
	@echo "Creating universal executable..."
	lipo -create -output trampoline_demo_universal $^
	@echo "Universal build complete: trampoline_demo_universal"

trampoline_demo_native: $(SRCS:.c=.o)
	$(CC) $(CFLAGS) -o $@ $^

trampoline_demo_x86_64: main.x86_64.o trampoline.x86_64.o
	$(CC) --target=x86_64-apple-darwin -o $@ $^

trampoline_demo_arm64: main.arm64.o trampoline.arm64.o
	$(CC) --target=arm64-apple-darwin -o $@ $^


# --- Object File Rules ---

# Native object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Position-independent object files (for shared libs)
%.shared.o: %.c
	$(CC) $(SHARED_CFLAGS) -c -o $@ $<

# Architecture-specific object files (for macOS universal builds)
%.x86_64.o: %.c
	$(CC) $(CFLAGS) --target=x86_64-apple-darwin -c -o $@ $<
%.arm64.o: %.c
	$(CC) $(CFLAGS) --target=arm64-apple-darwin -c -o $@ $<
%.x86_64.shared.o: %.c
	$(CC) $(SHARED_CFLAGS) --target=x86_64-apple-darwin -c -o $@ $<
%.arm64.shared.o: %.c
	$(CC) $(SHARED_CFLAGS) --target=arm64-apple-darwin -c -o $@ $<


# --- Utility Targets ---
clean:
	@echo "Cleaning up build artifacts..."
	rm -f trampoline_demo_* *.o *.a *.so *.dylib

.PHONY: all clean universal shared-lib static-lib help
