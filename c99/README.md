# minispdlog (C99)

A minimalistic, header-only, spdlog-like logging library for C99 and later.
This is the C99 port of the original C++ minispdlog library, designed to
provide efficient logging capabilities for C projects that require C99
compatibility.

## 🚀 Introduction

**minispdlog** is a lightweight, easy-to-integrate logging library inspired by
[spdlog](https://github.com/gabime/spdlog). It provides multiple log levels,
thread safety, optional asynchronous logging using a circular buffer, and
printf-style formatted messages. The library is implemented in a single header
file, making it ideal for embedding in C projects.

**Cross-Platform Support**: This library supports both Windows and POSIX
systems (Linux, macOS, Unix-like systems) with automatic platform detection and
appropriate API usage.

## 📜 Features

- **Header-only**: Single file inclusion with no external dependencies beyond
  system APIs
- **C99 Compatible**: Works with C99 standard and later
- **Cross-platform**: Supports Windows and POSIX systems
- **Log levels**: DEBUG, INFO, WARN, ERROR, CRITICAL
- **Thread-safe**: Uses platform-appropriate threading primitives
- **Asynchronous logging**: Optional async mode with circular buffer
- **Printf-style formatting**: Full support for printf format specifiers
- **Flexible output**: Log to stderr or files
- **Lightweight**: Minimal memory footprint and fast execution

## 🛠️ Requirements

- C99 compatible compiler (GCC, Clang, MSVC, etc.)
- Windows (Win32 API) or POSIX-compliant system (Linux, macOS, Unix-like)
- pthread library (POSIX systems only - automatically linked)

## 🚀 Usage

### 1. Include the header

```c
#include "minispdlog.h"
```

### 2. Initialize the logger

Before logging, initialize the logger with a filename (or NULL for stderr),
minimum log level, and async mode (0 for synchronous, 1 for asynchronous):

```c
logger_init("mylog.txt", INFO, 1);
```

### 3. Log messages

Use the provided functions for different log levels:

```c
// Simple string messages
logger_info("Application started");
logger_debug("Debug information");
logger_warn("Low disk space");
logger_error("Failed to open file");
logger_critical("Fatal error occurred");

// Formatted messages (printf-style)
logger_info_f("User %s logged in with ID %d", "alice", 123);
logger_debug_f("Processing item %d of %d", current, total);
logger_warn_f("Memory usage at %.1f%%", usage_percent);
logger_error_f("Connection failed after %d attempts", retry_count);
logger_critical_f("System overload: %d processes running", proc_count);
```

### 4. Change log level dynamically

You can change the minimum log level at runtime:

```c
logger_set_min_level(WARN);
```

### 5. Cleanup (important for async mode)

Always call deinit, especially when using async mode:

```c
logger_deinit();
```

## 🛒 Example

```c
#include <stdio.h>
#include "minispdlog.h"

int main() {
    // Initialize logger to write to file asynchronously
    logger_init("example.log", DEBUG, 1);

    logger_info("Application started");
    logger_debug_f("Debug value: %d", 42);
    logger_warn("This is a warning");
    logger_error_f("Failed to process file: %s", "data.txt");
    logger_critical("Critical system failure!");

    // Change log level
    logger_set_min_level(ERROR);
    logger_info("This won't appear");  // Below ERROR level
    logger_error("This will appear");

    logger_deinit();
    return 0;
}
```

## 🏗️ Building examples and tests

Use the provided Makefile:

```bash
make example  # Build example
make test     # Build test suite
make all      # Build both
make clean    # Clean build artifacts
make test-all # Run tests
```

## ⚙️ Configuration

### Log Levels

- `DEBUG`: Detailed information for debugging
- `INFO`: General information messages
- `WARN`: Warning messages for potentially harmful situations
- `ERROR`: Error messages for error conditions
- `CRITICAL`: Critical messages for very serious errors

### Async vs Sync Mode

- **Synchronous (async_mode = 0)**: Direct writes to file/stderr
- **Asynchronous (async_mode = 1)**: Background thread with circular buffer

## 🧪 Testing

Run the included test suite using the Makefile:

```bash
make test-all
```

## 📋 API Reference

- `void logger_init(const char* filename, LogLevel min_level, int async_mode);`
- `void logger_deinit(void);`
- `void logger_set_min_level(LogLevel level);`
- `void logger_debug(const char* message);`
- `void logger_info(const char* message);`
- `void logger_warn(const char* message);`
- `void logger_error(const char* message);`
- `void logger_critical(const char* message);`
- `void logger_debug_f(const char* format, ...);`
- `void logger_info_f(const char* format, ...);`
- `void logger_warn_f(const char* format, ...);`
- `void logger_error_f(const char* format, ...);`
- `void logger_critical_f(const char* format, ...);`

## 📄 License

MIT License - see the header file for full license text.

## 🔗 Related

- [Original C++ minispdlog](../): The C++ version of this library
- [spdlog](https://github.com/gabime/spdlog): The inspiration for this library
