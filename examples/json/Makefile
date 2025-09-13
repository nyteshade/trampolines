# Makefile for JSON Trampoline Example

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -O2 -g
INCLUDES = -I../../trampolines/include -I../..
LDFLAGS = -L../../lib
LIBS = -ltrampolines -ltrampoline

# Targets
DEMO = json_demo
ALL_TARGETS = $(DEMO)

# Default target
all: $(ALL_TARGETS)

# JSON demo program
$(DEMO): json_demo.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LIBS)

# Run the demo
run: $(DEMO)
	DYLD_LIBRARY_PATH=../../lib ./$(DEMO)

# Clean build artifacts
clean:
	rm -f $(ALL_TARGETS)
	rm -rf *.dSYM
	rm -f *.o

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(DEMO)
	DYLD_LIBRARY_PATH=../../lib lldb ./$(DEMO)

# Help target
help:
	@echo "JSON Trampoline Example Makefile"
	@echo "================================="
	@echo "Targets:"
	@echo "  all      - Build the JSON demo (default)"
	@echo "  run      - Build and run the JSON demo"
	@echo "  clean    - Remove build artifacts"
	@echo "  debug    - Build with debug symbols and run in debugger"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "The JSON class provides:"
	@echo "  - Full JSON parsing and generation"
	@echo "  - Object and array manipulation"
	@echo "  - Type-safe value access"
	@echo "  - Pretty printing support"
	@echo "  - Integration with String class"

.PHONY: all run clean debug help