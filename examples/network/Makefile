# Makefile for Trampoline Network Example
# Builds the network request/response example using libtrampolines

# Include SSL configuration
-include ../../Makefile.config

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -O2 -g -std=c99 $(SSL_CFLAGS)
INCLUDES = -I../../include
LDFLAGS = -L../../lib

# Libraries to link (include SSL libraries if enabled)
LIBS = -ltrampolines -ltrampoline $(SSL_LDFLAGS)

# Source files
DEMO_SRC = network_demo.c
SSL_DEMO_SRC = network_ssl_demo.c

# Output binaries
DEMO_TARGET = network_demo
SSL_DEMO_TARGET = network_ssl_demo
OLD_TARGET = network_test

# Default target
all: $(DEMO_TARGET) $(SSL_DEMO_TARGET)

# Build the new demo using libtrampolines
$(DEMO_TARGET): $(DEMO_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS)

# Build the SSL demo
$(SSL_DEMO_TARGET): $(SSL_DEMO_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS)

# Build the old test (for compatibility)
$(OLD_TARGET): network_example.c network_request.c network_response.c
	@echo "Note: Old network_test requires SSL libraries and old structure"
	@echo "Use 'make $(DEMO_TARGET)' for the new version using libtrampolines"

# Run the demo
run: $(DEMO_TARGET)
	./$(DEMO_TARGET)

# Clean build artifacts
clean:
	rm -f $(DEMO_TARGET) $(SSL_DEMO_TARGET) $(OLD_TARGET)
	rm -rf $(DEMO_TARGET).dSYM $(SSL_DEMO_TARGET).dSYM $(OLD_TARGET).dSYM

# Build with debug symbols (for debugging with gdb/lldb)
debug: CFLAGS += -DDEBUG -O0
debug: $(DEMO_TARGET)

# Build documentation (requires doxygen)
docs:
	@echo "Generating documentation..."
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen -g - | sed 's/PROJECT_NAME.*/PROJECT_NAME = "Network Trampoline Example"/' | \
		sed 's/INPUT.*/INPUT = ./' | \
		sed 's/EXTRACT_ALL.*/EXTRACT_ALL = YES/' | \
		doxygen - ; \
		echo "Documentation generated in html/index.html" ; \
	else \
		echo "Doxygen not found. Please install doxygen to generate docs." ; \
	fi

# Show help
help:
	@echo "Trampoline Network Example Makefile"
	@echo "===================================="
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build the network demo using libtrampolines (default)"
	@echo "  run     - Build and run the network demo"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build with debug symbols"
	@echo "  docs    - Generate Doxygen documentation"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "This example uses the modular trampoline libraries:"
	@echo "  - libtrampolines (classes: String, Network)"
	@echo "  - libtrampoline (core functionality)"
	@echo ""
	@echo "Build the libraries first:"
	@echo "  cd ../.. && make -f Makefile.classes all"
	@echo ""
	@echo "Examples:"
	@echo "  make          # Build the network demo"
	@echo "  make run      # Build and run the demo"
	@echo "  make clean    # Clean build artifacts"

.PHONY: all run clean debug docs help
