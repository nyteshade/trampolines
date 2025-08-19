# Makefile for the Trampoline Project

# --- Configuration ---
CC = clang
CFLAGS = -Wall -Wextra -std=c99 -g
# Flags for building Position-Independent Code for shared libraries
SHARED_CFLAGS = $(CFLAGS) -fPIC

# --- Installation Paths ---
# By default, install to /usr/local.
# Override with `make install PREFIX=$HOME/.local` for a user-local install.
PREFIX ?= /usr/local
INSTALL_LIB_DIR = $(PREFIX)/lib
INSTALL_INCLUDE_DIR = $(PREFIX)/include

# --- System Detection ---
OS = $(shell uname)
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

# The default target builds the main executable.
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
	@echo "Main Targets:"
	@echo "  all                  Builds the default executable for the platform."
	@echo "  help                 Shows this help message."
	@echo ""
	@echo "Library Targets:"
	@echo "  shared-lib           Builds a shared library (.dylib or .so)."
	@echo "  static-lib           Builds a static library (.a)."
	@echo ""
	@echo "Installation Targets:"
	@echo "  install              Install headers and libraries. Use 'sudo' for system paths."
	@echo "                       (e.g., 'make install PREFIX=$$HOME/.local' for user install)"
	@echo "  uninstall            Remove installed headers and libraries."
	@echo ""
	@echo "macOS Specific:"
	@echo "  universal            Builds a universal executable (x86_64 and arm64)."
	@echo ""
	@echo "Utility Targets:"
	@echo "  clean                Removes all build artifacts."

# --- Installation Targets ---

install: shared-lib static-lib
	@echo "Installing to $(PREFIX)..."
	mkdir -p $(INSTALL_LIB_DIR)
	mkdir -p $(INSTALL_INCLUDE_DIR)
	install -m 644 trampoline.h $(INSTALL_INCLUDE_DIR)
	install -m 644 $(SHARED_LIB) $(INSTALL_LIB_DIR)
	install -m 644 $(STATIC_LIB) $(INSTALL_LIB_DIR)
	@echo "Installation complete."

uninstall:
	@echo "Uninstalling from $(PREFIX)..."
	rm -f $(INSTALL_INCLUDE_DIR)/trampoline.h
	rm -f $(INSTALL_LIB_DIR)/$(SHARED_LIB)
	rm -f $(INSTALL_LIB_DIR)/$(STATIC_LIB)
	@echo "Uninstallation complete."


# --- Library Targets ---
shared-lib: $(SHARED_LIB)
static-lib: $(STATIC_LIB)

# macOS (Darwin) specific library builds
ifeq ($(OS), Darwin)
$(SHARED_LIB): libtrampoline.x86_64.dylib libtrampoline.arm64.dylib
	@echo "Creating universal shared library..."
	lipo -create -output $@ $^
$(STATIC_LIB): libtrampoline.x86_64.a libtrampoline.arm64.a
	@echo "Creating universal static library..."
	lipo -create -output $@ $^
libtrampoline.x86_64.dylib: trampoline.x86_64.shared.o
	$(CC) --target=x86_64-apple-darwin -dynamiclib -o $@ $^
libtrampoline.arm64.dylib: trampoline.arm64.shared.o
	$(CC) --target=arm64-apple-darwin -dynamiclib -o $@ $^
libtrampoline.x86_64.a: trampoline.x86_64.o
	ar rcs $@ $^
libtrampoline.arm64.a: trampoline.arm64.o
	ar rcs $@ $^
# Linux specific library builds
else
$(SHARED_LIB): $(SHARED_LIB_SRC:.c=.shared.o)
	@echo "Building Linux shared library..."
	$(CC) $(SHARED_CFLAGS) -shared -o $@ $^
$(STATIC_LIB): $(STATIC_LIB_SRC:.c=.o)
	@echo "Building Linux static library..."
	ar rcs $@ $^
endif

# --- Executable Targets ---
universal: trampoline_demo_x86_64 trampoline_demo_arm64
	@echo "Creating universal executable..."
	lipo -create -output trampoline_demo_universal $^
trampoline_demo_native: $(SRCS:.c=.o)
	$(CC) $(CFLAGS) -o $@ $^
trampoline_demo_x86_64: main.x86_64.o trampoline.x86_64.o
	$(CC) --target=x86_64-apple-darwin -o $@ $^
trampoline_demo_arm64: main.arm64.o trampoline.arm64.o
	$(CC) --target=arm64-apple-darwin -o $@ $^

# --- Object File Rules ---
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.shared.o: %.c
	$(CC) $(SHARED_CFLAGS) -c -o $@ $<
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

.PHONY: all clean universal shared-lib static-lib help install uninstall
