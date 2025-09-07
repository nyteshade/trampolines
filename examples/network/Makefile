# Makefile for Trampoline Network Example
# Builds the network request/response example using trampolines

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -lssl -lcrypto
INCLUDES = -I../.. -I. -I/opt/local/include -L/opt/local/lib

# Source files
TRAMPOLINE_SRCS = ../../trampoline.c ../../trampoline_helpers.c
EXAMPLE_SRCS = network_example.c network_request.c network_response.c

# Output binary
TARGET = network_test

# Default target
all: $(TARGET)

# Build the network test
$(TARGET): $(EXAMPLE_SRCS) $(TRAMPOLINE_SRCS) \
           network_request.h network_response.h network_response_private.h \
           network_request.c network_response.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) \
	      $(EXAMPLE_SRCS) $(TRAMPOLINE_SRCS)

# Run the test
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)
	rm -rf $(TARGET).dSYM

# Build with debug symbols (for debugging with gdb/lldb)
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

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
	@echo "  all     - Build the network test (default)"
	@echo "  run     - Build and run the network test"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build with debug symbols"
	@echo "  docs    - Generate Doxygen documentation"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make          # Build the network test"
	@echo "  make run      # Build and run the test"
	@echo "  make clean    # Clean build artifacts"

.PHONY: all run clean debug docs help
