# minispdlog

A minimalistic, header-only, spdlog-like logger for C++14 and later. It is
inspired by [spdlog](https://github.com/gabime/spdlog).  I wrote it because in
a project I was working on, I needed a logger for an old Centos 7 system, which
couldn't be updgraded to support modern libraries. I was not able to install
the original `spdlog` library there, so I was forced to write my own. This is
the result.

## 🚀 Introduction

**minispdlog** is a lightweight, easy-to-integrate logging library inspired by
*[spdlog]. It provides multiple log levels, thread safety, optional asynchronous
*logging, and simple printf-style formatting. The library is implemented in a
*single header file, making it ideal for embedding in small projects or for
*quick prototyping.

## 📜 Features

- Header-only, no dependencies except the C++ standard library
- Log levels: DEBUG, INFO, WARN, ERROR, CRITICAL
- Thread-safe logging
- Optional asynchronous logging mode
- Simple `{}`-based formatting for log messages
- Macros for convenient logging
- Timestamp and thread ID included in each log entry

## 🚀 Usage

### 1. Add the header

Copy `minispdlog.h` into your project and include it:

```cpp
#include "minispdlog.h"
```

### 2. Initialize the logger

Before logging, initialize the logger with a file name, minimum log level, and (optionally) async mode:

```cpp
MiniLogger::LoggerManager::initialize("mylog.txt", MiniLogger::LogLevel::DEBUG, false);
// Or for async logging:
MiniLogger::LoggerManager::initialize("mylog.txt", MiniLogger::LogLevel::INFO, true);
```

### 3. Log messages

You can use the provided macros or call the logger directly:

```cpp
SLOG_INFO("Application started");
SLOG_DEBUG("Debug value: {}", 42);
SLOG_WARN("Low disk space: {} MB left", 128);
SLOG_ERROR("Failed to open file: {}", "data.txt");
SLOG_CRITICAL("Fatal error: {}", "Out of memory");

// Or use the logger instance directly:
auto& logger = MiniLogger::LoggerManager::get();
logger.info("Direct info message");
logger.warn("Direct warning: {}", "something happened");
```

### 4. Shutdown (optional)

To clean up resources (especially in async mode), call:

```cpp
MiniLogger::LoggerManager::shutdown();
```

## 🛒 Example

```cpp
#include "minispdlog.h"

int main() {
    MiniLogger::LoggerManager::initialize("example.log", MiniLogger::LogLevel::DEBUG);

    SLOG_INFO("Starting application");
    SLOG_DEBUG("Debug value: {}", 123);
    SLOG_WARN("This is a warning");
    SLOG_ERROR("An error occurred: {}", "file not found");
    SLOG_CRITICAL("Critical failure!");

    MiniLogger::LoggerManager::shutdown();
    return 0;
}
```

## Log Output Example

```text
2025-05-23 12:16:08.907630 [DEBUG] [Thread:758] This is a debug message
2025-05-23 12:16:08.907650 [WARN] [Thread:758] This is a warning message
2025-05-23 12:16:08.907663 [ERROR] [Thread:758] This is an error message
2025-05-23 12:16:08.907687 [INFO] [Thread:758] User Alice logger in with ID 12345
2025-05-23 12:16:08.907702 [WARN] [Thread:758] Direct warning message
```
