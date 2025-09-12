# Makefile for String Trampoline Example
# Uses the libtrampolines library

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -O2 -g
CFLAGS_C89 = -Wall -Wextra -std=c89 -pedantic -g
INCLUDES = -I../../trampolines/include -I../..
LDFLAGS = -L../../lib
LIBS = -ltrampolines -ltrampoline

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

# Default target
all: $(ALL_TARGETS)

# Main demo program
$(TARGET): string_demo.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS)

# C89-compliant demo
$(TARGET_C89): string_demo_c89.c
	$(CC) $(CFLAGS_C89) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS)

# Simple test program
$(SIMPLE_TEST): simple_string_test.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS)

# Performance test program
$(PERF_TEST): string_performance.c
	$(CC) $(CFLAGS) -O2 $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS)

# String example program
$(EXAMPLE): string_example.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS)

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