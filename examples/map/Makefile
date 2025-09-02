# Makefile for Trampoline Map v2 with MapNode Integration
# Builds the complete zero-cognitive-load Map system

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -std=c99
INCLUDES = -I../.. -I.

# Source files
TRAMPOLINE_SRCS = ../../trampoline.c ../../trampoline_helpers.c
EXAMPLE_SRCS = map_example.c

# Output binaries
MAIN_TARGET = map_test
MAPNODE_TARGET = mapnode_test
USAGE_TARGET = usage_example
SIMPLE_TARGET = simple_map_test
DEBUG_TARGET = debug_map
MINIMAL_TARGET = minimal_map

# All sample apps
ALL_TARGETS = $(MAIN_TARGET) $(MAPNODE_TARGET) $(USAGE_TARGET) $(SIMPLE_TARGET) $(DEBUG_TARGET) $(MINIMAL_TARGET)

# Default target - build all sample apps
all: $(ALL_TARGETS)

# Build the main map test with MapNode integration
$(MAIN_TARGET): $(EXAMPLE_SRCS) $(TRAMPOLINE_SRCS) \
                map.h map_impl.c mapnode.h mapnode_impl.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN_TARGET) \
	      $(EXAMPLE_SRCS) $(TRAMPOLINE_SRCS)

# Build standalone MapNode test
$(MAPNODE_TARGET): mapnode_test.c $(TRAMPOLINE_SRCS) mapnode.h mapnode_impl.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAPNODE_TARGET) \
	      mapnode_test.c $(TRAMPOLINE_SRCS)

# Build usage examples
$(USAGE_TARGET): usage_example.c $(TRAMPOLINE_SRCS) mapnode.h mapnode_impl.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(USAGE_TARGET) \
	      usage_example.c $(TRAMPOLINE_SRCS)

# Build simple map test
$(SIMPLE_TARGET): simple_map_test.c $(TRAMPOLINE_SRCS) map.h map_impl.c mapnode.h mapnode_impl.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(SIMPLE_TARGET) \
	      simple_map_test.c $(TRAMPOLINE_SRCS)

# Build debug map example
$(DEBUG_TARGET): debug_map.c $(TRAMPOLINE_SRCS) map.h map_impl.c mapnode.h mapnode_impl.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(DEBUG_TARGET) \
	      debug_map.c $(TRAMPOLINE_SRCS)

# Build minimal map example
$(MINIMAL_TARGET): minimal_map.c $(TRAMPOLINE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MINIMAL_TARGET) \
	      minimal_map.c $(TRAMPOLINE_SRCS)

# Run the main test
run: $(MAIN_TARGET)
	./$(MAIN_TARGET)

# Run MapNode tests
test-mapnode: $(MAPNODE_TARGET)
	./$(MAPNODE_TARGET)

# Run usage examples
demo: $(USAGE_TARGET)
	./$(USAGE_TARGET)

# Run simple map test
test-simple: $(SIMPLE_TARGET)
	./$(SIMPLE_TARGET)

# Run debug map example
test-debug: $(DEBUG_TARGET)
	./$(DEBUG_TARGET)

# Run minimal map example  
test-minimal: $(MINIMAL_TARGET)
	./$(MINIMAL_TARGET)

# Run all tests
test-all: $(ALL_TARGETS)
	@echo "=== Running MapNode Tests ==="
	./$(MAPNODE_TARGET)
	@echo ""
	@echo "=== Running Usage Examples ==="
	./$(USAGE_TARGET)
	@echo ""
	@echo "=== Running Simple Map Tests ==="
	./$(SIMPLE_TARGET)
	@echo ""
	@echo "=== Running Debug Map Example ==="
	./$(DEBUG_TARGET)
	@echo ""
	@echo "=== Running Minimal Map Example ==="
	./$(MINIMAL_TARGET)
	@echo ""
	@echo "=== Running Complete Map Tests ==="
	./$(MAIN_TARGET)

# Clean build artifacts
clean:
	rm -f $(ALL_TARGETS) map_example_c89
	rm -rf $(MAIN_TARGET).dSYM $(MAPNODE_TARGET).dSYM $(USAGE_TARGET).dSYM \
	       $(SIMPLE_TARGET).dSYM $(DEBUG_TARGET).dSYM $(MINIMAL_TARGET).dSYM \
	       map_example_c89.dSYM
	rm -rf html/  # Remove doxygen documentation if present

# Build with debug symbols (for debugging with gdb/lldb)
debug: CFLAGS += -DDEBUG -O0
debug: $(MAIN_TARGET)

# Build documentation (requires doxygen)
docs:
	@echo "Generating documentation..."
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen -g - | sed 's/PROJECT_NAME.*/PROJECT_NAME = "Map v2 with MapNode Integration"/' | \
		sed 's/INPUT.*/INPUT = ./' | \
		sed 's/EXTRACT_ALL.*/EXTRACT_ALL = YES/' | \
		sed 's/RECURSIVE.*/RECURSIVE = NO/' | \
		doxygen - ; \
		echo "Documentation generated in html/index.html" ; \
	else \
		echo "Doxygen not found. Please install doxygen to generate docs." ; \
	fi

# Performance benchmarking
benchmark: $(MAIN_TARGET)
	@echo "Running performance benchmark..."
	time ./$(MAIN_TARGET) > /dev/null

# Memory debugging with valgrind (if available)
memcheck: $(MAIN_TARGET)
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --leak-check=full --show-leak-kinds=all ./$(MAIN_TARGET); \
	else \
		echo "Valgrind not found. Running with lldb instead..."; \
		lldb ./$(MAIN_TARGET); \
	fi

# Show help
help:
	@echo "Trampoline Map v2 with MapNode Integration"
	@echo "=========================================="
	@echo ""
	@echo "This Makefile builds the complete zero-cognitive-load Map system"
	@echo "with MapNode integration for ultimate type safety and usability."
	@echo ""
	@echo "Targets:"
	@echo "  all           - Build all sample applications (default)"
	@echo "  run           - Build and run the complete map test"
	@echo "  test-mapnode  - Build and run MapNode-only tests"
	@echo "  demo          - Build and run usage examples"
	@echo "  test-simple   - Build and run simple map test"
	@echo "  test-debug    - Build and run debug map example"
	@echo "  test-minimal  - Build and run minimal map example"
	@echo "  test-all      - Run all sample applications in sequence"
	@echo "  clean         - Remove all build artifacts and dSYM directories"
	@echo "  debug         - Build with debug symbols"
	@echo "  docs          - Generate Doxygen documentation"
	@echo "  benchmark     - Run performance benchmark"
	@echo "  memcheck      - Run with memory debugging"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make run      # Build and run complete system test"
	@echo "  make test-all # Run comprehensive test suite"
	@echo "  make demo     # See MapNode usage examples"
	@echo "  make clean    # Clean up build artifacts"
	@echo ""
	@echo "Features demonstrated:"
	@echo "  • Zero cognitive load API (no size management)"
	@echo "  • Type-safe operations with runtime validation"
	@echo "  • Mixed type support (any MapNode as key/value)"
	@echo "  • Comprehensive error handling and debugging"
	@echo "  • Performance optimization with auto-resizing"
	@echo "  • Memory introspection with magic byte validation"

.PHONY: all run test-mapnode demo test-simple test-debug test-minimal test-all clean debug docs benchmark memcheck help