# Cross-platform Makefile for minispdlog
CC = gcc
CFLAGS = -Wall -g -std=c99 -pedantic

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(OS),Windows_NT)
    # Windows (MinGW/MSYS2)
    PLATFORM = WINDOWS
    LIBS = 
    CFLAGS += -D_WIN32
else
    # POSIX (Linux, macOS, etc.)
    PLATFORM = POSIX
    LIBS = -lpthread
    CFLAGS += -D_POSIX_C_SOURCE=200809L
endif

# Targets
.PHONY: all clean test-all platform-info

all: example test platform-info

# Main targets
example: example.c minispdlog.h
	$(CC) $(CFLAGS) -o example example.c $(LIBS)

test: test.c minispdlog.h
	$(CC) $(CFLAGS) -o test test.c $(LIBS)

# Run tests
test-all: test
	@echo "Running tests..."
	./test

# Show platform information
platform-info:
	@echo "Build Configuration:"
	@echo "  Platform: $(PLATFORM)"
	@echo "  Compiler: $(CC)"
	@echo "  Flags: $(CFLAGS)"
	@echo "  Libraries: $(LIBS)"

clean:
	rm -f example test
	rm -f *.log *.o

# Windows-specific instructions (if running on Windows with MinGW)
windows-build:
	@echo "For Windows with MinGW/MSYS2:"
	@echo "  gcc -Wall -g -std=c99 -o example_crossplatform.exe example_crossplatform.c"
	@echo "  gcc -Wall -g -std=c99 -o test_crossplatform.exe test_crossplatform.c"

help:
	@echo "Available targets:"
	@echo "  all                 - Build all examples and tests"
	@echo "  example             - Build original POSIX example"
	@echo "  test                - Build original POSIX test"
	@echo "  example_crossplatform - Build cross-platform example"
	@echo "  test_crossplatform  - Build cross-platform test"
	@echo "  test-all            - Run all tests"
	@echo "  platform-info       - Show build configuration"
	@echo "  windows-build       - Show Windows build instructions"
	@echo "  clean               - Remove all build artifacts"
	@echo "  help                - Show this help message"
