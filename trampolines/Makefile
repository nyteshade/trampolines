# Makefile for Trampoline Classes Library (libtrampolines)
# This builds the optional classes that use the core trampoline library
# Requires libtrampoline to be built first (run make in parent directory)

# Include SSL configuration
-include ../Makefile.config

# Compiler and flags
CC = gcc
AR = ar
CFLAGS = -Wall -O2 -fPIC $(SSL_CFLAGS) -I../
LDFLAGS = -shared

# Detect OS for library extension
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    DYLIB_EXT = dylib
    LDFLAGS = -dynamiclib
else
    DYLIB_EXT = so
endif

# Directories
INCLUDE_DIR = include
SRC_DIR = src
CLASSES_DIR = $(SRC_DIR)/classes
LIB_DIR = ../lib
INSTALL_PREFIX = /usr/local
INSTALL_LIB_DIR = $(INSTALL_PREFIX)/lib
INSTALL_INC_DIR = $(INSTALL_PREFIX)/include

# Classes library files
CLASSES_SRCS = $(CLASSES_DIR)/string.c \
               $(CLASSES_DIR)/network_common.c \
               $(CLASSES_DIR)/network_request.c \
               $(CLASSES_DIR)/network_response.c \
               $(CLASSES_DIR)/json.c

CLASSES_OBJS = $(CLASSES_SRCS:.c=.o)
CLASSES_LIB_STATIC = $(LIB_DIR)/libtrampolines.a
CLASSES_LIB_SHARED = $(LIB_DIR)/libtrampolines.$(DYLIB_EXT)

# Headers to install
CLASSES_HEADERS = $(INCLUDE_DIR)/trampolines/string.h \
                  $(INCLUDE_DIR)/trampolines/network.h \
                  $(INCLUDE_DIR)/trampolines/json.h

# Default target
all: $(CLASSES_LIB_STATIC) $(CLASSES_LIB_SHARED)

# Create lib directory
$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# Build static library
$(CLASSES_LIB_STATIC): $(CLASSES_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^
	@echo "Built static classes library: $@"

# Build shared library
$(CLASSES_LIB_SHARED): $(CLASSES_OBJS) | $(LIB_DIR)
ifeq ($(UNAME_S),Darwin)
	$(CC) $(LDFLAGS) -install_name @rpath/libtrampolines.$(DYLIB_EXT) \
		-L$(LIB_DIR) -ltrampoline $(SSL_LDFLAGS) -o $@ $(CLASSES_OBJS)
else
	$(CC) $(LDFLAGS) -L$(LIB_DIR) -ltrampoline $(SSL_LDFLAGS) -o $@ $(CLASSES_OBJS)
endif
	@echo "Built shared classes library: $@"
	@echo "Note: Link with -ltrampoline -ltrampolines"

# Pattern rules for object files
$(CLASSES_DIR)/%.o: $(CLASSES_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Specific rules with dependencies
$(CLASSES_DIR)/string.o: $(CLASSES_DIR)/string.c $(INCLUDE_DIR)/trampolines/string.h
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(CLASSES_DIR)/network_common.o: $(CLASSES_DIR)/network_common.c $(CLASSES_DIR)/network_common.h
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(CLASSES_DIR)/network_request.o: $(CLASSES_DIR)/network_request.c $(INCLUDE_DIR)/trampolines/network.h $(CLASSES_DIR)/network_common.h
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(CLASSES_DIR)/network_response.o: $(CLASSES_DIR)/network_response.c $(INCLUDE_DIR)/trampolines/network.h
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(CLASSES_DIR)/json.o: $(CLASSES_DIR)/json.c $(INCLUDE_DIR)/trampolines/json.h $(INCLUDE_DIR)/trampolines/string.h
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Installation
install: all
	@echo "Installing classes library..."
	install -d $(INSTALL_INC_DIR)/trampolines
	for header in $(CLASSES_HEADERS); do \
		if [ -f $$header ]; then \
			install -m 644 $$header $(INSTALL_INC_DIR)/trampolines/; \
		fi \
	done
	install -m 644 $(CLASSES_LIB_STATIC) $(INSTALL_LIB_DIR)/
	install -m 755 $(CLASSES_LIB_SHARED) $(INSTALL_LIB_DIR)/
ifeq ($(UNAME_S),Darwin)
	@echo "Updating dylib install names..."
	install_name_tool -id $(INSTALL_LIB_DIR)/libtrampolines.$(DYLIB_EXT) \
		$(INSTALL_LIB_DIR)/libtrampolines.$(DYLIB_EXT)
endif
	@echo "Classes library installed to $(INSTALL_LIB_DIR), includes to $(INSTALL_INC_DIR)"

# Uninstall
uninstall:
	@echo "Uninstalling classes library..."
	rm -f $(INSTALL_INC_DIR)/trampolines/*.h
	rm -f $(INSTALL_LIB_DIR)/libtrampolines.a
	rm -f $(INSTALL_LIB_DIR)/libtrampolines.$(DYLIB_EXT)
	rmdir $(INSTALL_INC_DIR)/trampolines 2>/dev/null || true

# Clean
clean:
	rm -f $(CLASSES_DIR)/*.o
	rm -f $(CLASSES_LIB_STATIC) $(CLASSES_LIB_SHARED)

# Individual class targets for testing
string-only: $(CLASSES_DIR)/string.o
	$(AR) rcs $(LIB_DIR)/libtrampoline_string.a $<
	@echo "Built string-only library"

network-only: $(CLASSES_DIR)/network_common.o $(CLASSES_DIR)/network_request.o $(CLASSES_DIR)/network_response.o
	$(AR) rcs $(LIB_DIR)/libtrampoline_network.a $^
	@echo "Built network-only library"

# SSL configuration info
ssl-info:
	@$(MAKE) -f ../Makefile.config show

# Help
help:
	@echo "Trampoline Classes Library Makefile"
	@echo "===================================="
	@echo ""
	@echo "Build targets:"
	@echo "  make              - Build classes library (libtrampolines)"
	@echo "  make string-only  - Build only String class"
	@echo "  make network-only - Build only Network classes"
	@echo ""
	@echo "Installation:"
	@echo "  make install      - Install classes library"
	@echo "  make uninstall    - Uninstall classes library"
	@echo ""
	@echo "Other:"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make ssl-info     - Show SSL configuration"
	@echo "  make help         - Show this help"
	@echo ""
	@echo "Current classes in libtrampolines:"
	@echo "  - String  (trampolines/string.h)"
	@echo "  - Network (trampolines/network.h) with SSL support"
	@echo "  - Json    (trampolines/json.h)"
	@echo ""
	@echo "Usage:"
	@echo "  #include <trampolines/string.h>"
	@echo "  #include <trampolines/network.h>"
	@echo "  #include <trampolines/json.h>"
	@echo "  gcc prog.c -ltrampoline -ltrampolines $(SSL_LDFLAGS)"

.PHONY: all install uninstall clean string-only network-only ssl-info help
