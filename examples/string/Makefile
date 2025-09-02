# Makefile for String Trampoline Example
# Demonstrates comprehensive string manipulation with member functions

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -std=c89 -pedantic -g
LDFLAGS = 

# Platform detection
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Targets
TARGET = string_demo
SIMPLE_TEST = simple_string_test
PERF_TEST = string_performance
EXAMPLE = string_example
ALL_TARGETS = $(TARGET) $(SIMPLE_TEST) $(PERF_TEST)

# Source files
TRAMPOLINE_SRC = ../../trampoline.c
STRING_IMPL = string_impl.c
EXAMPLE_SRC = string_example.c

# Default target
all: $(ALL_TARGETS)

# Main demo program (C89 compliant)
$(TARGET): string_demo.c string.h $(STRING_IMPL) $(TRAMPOLINE_SRC)
	$(CC) $(CFLAGS) -o $@ string_demo.c $(TRAMPOLINE_SRC) $(LDFLAGS)

# Simple test program
$(SIMPLE_TEST): simple_string_test.c string.h $(STRING_IMPL) $(TRAMPOLINE_SRC)
	$(CC) $(CFLAGS) -o $@ simple_string_test.c $(TRAMPOLINE_SRC) $(LDFLAGS)

# Performance test program
$(PERF_TEST): string_performance.c string.h $(STRING_IMPL) $(TRAMPOLINE_SRC)
	$(CC) $(CFLAGS) -O2 -o $@ string_performance.c $(TRAMPOLINE_SRC) $(LDFLAGS)

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
	@echo "  all         - Build all example programs (default)"
	@echo "  run         - Build and run the main example"
	@echo "  test-simple - Run simple string test"
	@echo "  test-perf   - Run performance test"
	@echo "  test-all    - Run all tests"
	@echo "  clean       - Remove build artifacts"
	@echo "  debug       - Build with debug symbols and run in debugger"
	@echo "  docs        - Generate documentation (requires doxygen)"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Platform: $(UNAME_S) $(UNAME_M)"
	@echo "Compiler: $(CC)"

.PHONY: all run test-simple test-perf test-all clean debug docs help