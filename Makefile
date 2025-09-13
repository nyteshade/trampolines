# Main Makefile for Trampoline Core Library
# This builds only the core trampoline library (libtrampoline)
# For the optional classes library, see trampolines/Makefile

# Compiler and flags
CC = gcc
AR = ar
CFLAGS = -Wall -O2 -fPIC
LDFLAGS = -shared

# Detect OS for library extension
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    DYLIB_EXT = dylib
    LDFLAGS = -dynamiclib
else
    DYLIB_EXT = so
endif

# Detect architecture for source selection
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
    ARCH_SRC = trampoline_x86_64.c
else ifeq ($(UNAME_M),arm64)
    ARCH_SRC = trampoline_arm64.c
else ifeq ($(UNAME_M),aarch64)
    ARCH_SRC = trampoline_arm64.c
else
    $(error Unsupported architecture: $(UNAME_M))
endif

# Directories
LIB_DIR = lib
INSTALL_PREFIX = /usr/local

# Core library files
CORE_SRCS = $(ARCH_SRC) trampoline_helpers.c
CORE_OBJS = $(CORE_SRCS:.c=.o)
CORE_LIB_STATIC = $(LIB_DIR)/libtrampoline.a
CORE_LIB_SHARED = $(LIB_DIR)/libtrampoline.$(DYLIB_EXT)

# Default target
all: $(CORE_LIB_STATIC) $(CORE_LIB_SHARED)

# Create lib directory
$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# Build static library
$(CORE_LIB_STATIC): $(CORE_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^
	@echo "Built static core library: $@"

# Build shared library
$(CORE_LIB_SHARED): $(CORE_OBJS) | $(LIB_DIR)
ifeq ($(UNAME_S),Darwin)
	$(CC) $(LDFLAGS) -install_name @rpath/libtrampoline.$(DYLIB_EXT) -o $@ $^
else
	$(CC) $(LDFLAGS) -o $@ $^
endif
	@echo "Built shared core library: $@"

# Pattern rule for object files
%.o: %.c trampoline.h
	$(CC) $(CFLAGS) -c $< -o $@

# Installation
install: all
	@echo "Installing core trampoline library..."
	install -d $(INSTALL_PREFIX)/include
	install -d $(INSTALL_PREFIX)/lib
	install -m 644 trampoline.h $(INSTALL_PREFIX)/include/
	install -m 644 $(CORE_LIB_STATIC) $(INSTALL_PREFIX)/lib/
	install -m 755 $(CORE_LIB_SHARED) $(INSTALL_PREFIX)/lib/
ifeq ($(UNAME_S),Darwin)
	@echo "Updating dylib install names..."
	install_name_tool -id $(INSTALL_PREFIX)/lib/libtrampoline.$(DYLIB_EXT) \
		$(INSTALL_PREFIX)/lib/libtrampoline.$(DYLIB_EXT)
endif
	@echo "Core library installed to $(INSTALL_PREFIX)"

# Uninstall
uninstall:
	@echo "Uninstalling core library..."
	rm -f $(INSTALL_PREFIX)/include/trampoline.h
	rm -f $(INSTALL_PREFIX)/lib/libtrampoline.a
	rm -f $(INSTALL_PREFIX)/lib/libtrampoline.$(DYLIB_EXT)

# Clean
clean:
	rm -f *.o
	rm -rf $(LIB_DIR)

# Build all (core + classes)
all-with-classes: all
	@echo "Building classes library..."
	$(MAKE) -C trampolines all

# Install all (core + classes)
install-all: install
	$(MAKE) -C trampolines install

# Help
help:
	@echo "Trampoline Core Library Makefile"
	@echo "================================="
	@echo ""
	@echo "Core library targets:"
	@echo "  make              - Build core library only"
	@echo "  make install      - Install core library"
	@echo "  make uninstall    - Uninstall core library"
	@echo "  make clean        - Remove build artifacts"
	@echo ""
	@echo "Combined targets (core + classes):"
	@echo "  make all-with-classes    - Build core and classes libraries"
	@echo "  make install-all         - Install everything"
	@echo ""
	@echo "For classes library only:"
	@echo "  cd trampolines && make"
	@echo ""
	@echo "Current architecture: $(UNAME_M) -> $(ARCH_SRC)"

.PHONY: all install uninstall clean all-with-classes install-all help