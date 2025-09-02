# Makefile for String Trampoline Example
# Demonstrates comprehensive string manipulation with member functions

# Compiler and flags
CC = cc
# Default flags - no strict C89 to support all architectures
CFLAGS = -Wall -Wextra -g
# Strict C89 flags for testing compatibility
CFLAGS_C89 = -Wall -Wextra -std=c89 -pedantic -g
LDFLAGS = 

# Platform detection
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Targets
TARGET = string_demo
TARGET_C89 = string_demo_c89
SIMPLE_TEST = simple_string_test
PERF_TEST = string_performance
EXAMPLE = string_example
ALL_TARGETS = $(TARGET) $(TARGET_C89) $(SIMPLE_TEST) $(PERF_TEST)

# Source files
# Use the umbrella trampoline.c which automatically includes the correct
# architecture-specific implementation based on the platform
TRAMPOLINE_SRC = ../../trampoline.c
TRAMPOLINE_HELPERS = ../../trampoline_helpers.c
STRING_IMPL = string_impl.c
EXAMPLE_SRC = string_example.c

# Default target
all: $(ALL_TARGETS)

# Main demo program
$(TARGET): string_demo.c string.h $(STRING_IMPL) $(TRAMPOLINE_SRC) $(TRAMPOLINE_HELPERS)
	$(CC) $(CFLAGS) -o $@ string_demo.c $(TRAMPOLINE_SRC) $(TRAMPOLINE_HELPERS) $(LDFLAGS)

# C89-compliant demo (with portable printf format specifiers)
$(TARGET_C89): string_demo_c89.c string.h $(STRING_IMPL) $(TRAMPOLINE_SRC) $(TRAMPOLINE_HELPERS)
	$(CC) $(CFLAGS) -o $@ string_demo_c89.c $(TRAMPOLINE_SRC) $(TRAMPOLINE_HELPERS) $(LDFLAGS)

# Simple test program
$(SIMPLE_TEST): simple_string_test.c string.h $(STRING_IMPL) $(TRAMPOLINE_SRC) $(TRAMPOLINE_HELPERS)
	$(CC) $(CFLAGS) -o $@ simple_string_test.c $(TRAMPOLINE_SRC) $(TRAMPOLINE_HELPERS) $(LDFLAGS)

# Performance test program
$(PERF_TEST): string_performance.c string.h $(STRING_IMPL) $(TRAMPOLINE_SRC) $(TRAMPOLINE_HELPERS)
	$(CC) $(CFLAGS) -O2 -o $@ string_performance.c $(TRAMPOLINE_SRC) $(TRAMPOLINE_HELPERS) $(LDFLAGS)

# Run the main example
run: $(TARGET)
	./$(TARGET)

# Run simple test
test-simple: $(SIMPLE_TEST)
	./$(SIMPLE_TEST)

# Run performance test
test-perf: $(PERF_TEST)
	./$(PERF_TEST)

# Run all tests
test-all: run test-simple test-perf

# Clean build artifacts
clean:
	rm -f $(ALL_TARGETS)
	rm -rf *.dSYM
	rm -f *.o

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)
	lldb ./$(TARGET)

# Build with strict C89 compliance checking (may fail on some architectures)
c89-strict: CFLAGS = $(CFLAGS_C89)
c89-strict: $(TARGET_C89)
	@echo "Built with strict C89 compliance checking"

# Build documentation
docs:
	@echo "Generating documentation..."
	@doxygen -g Doxyfile 2>/dev/null || true
	@doxygen Doxyfile 2>/dev/null || echo "Doxygen not installed"

# Help target
help:
	@echo "String Trampoline Example Makefile"
	@echo "=================================="
	@echo "Targets:"
	@echo "  all           - Build all example programs (default)"
	@echo "  string_demo   - Build main demo program"
	@echo "  string_demo_c89 - Build C89-compliant demo (portable printf)"
	@echo "  run           - Build and run the main demo"
	@echo "  test-simple   - Run simple string test"
	@echo "  test-perf     - Run performance test"
	@echo "  test-all      - Run all tests"
	@echo "  clean         - Remove build artifacts"
	@echo "  debug         - Build with debug symbols and run in debugger"
	@echo "  c89-strict    - Build with strict C89 checking (may fail on some arch)"
	@echo "  docs          - Generate documentation (requires doxygen)"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Notes:"
	@echo "  - Uses umbrella trampoline.c for automatic architecture detection"
	@echo "  - Includes trampoline_helpers.c for allocation tracking"
	@echo "  - C89 flag: -std=c89 for maximum compatibility"
	@echo ""
	@echo "Platform: $(UNAME_S) $(UNAME_M)"
	@echo "Compiler: $(CC)"

.PHONY: all run test-simple test-perf test-all clean debug docs help