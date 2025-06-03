# CMake Build Guide for minispdlog

This document describes how to build and use the minispdlog library with CMake.

## Quick Start

### Building with CMake

```bash
# Create build directory
mkdir build && cd build

# Configure (basic)
cmake ..

# Configure with options
cmake -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON -DENABLE_WARNINGS=ON ..

# Build
cmake --build .

# Install (optional)
cmake --install . --prefix /usr/local
```

### Windows-specific

```cmd
# Using Visual Studio
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release

# Using MinGW
cmake -G "MinGW Makefiles" ..
cmake --build .
```

### Linux/macOS

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_EXAMPLES` | ON | Build example programs |
| `BUILD_TESTS` | ON | Build test programs |
| `ENABLE_WARNINGS` | ON | Enable compiler warnings |

## Custom Targets

```bash
# Run example program
cmake --build . --target run-example

# Run tests
cmake --build . --target run-tests

# Clean log files
cmake --build . --target clean-logs

# Run CTest
ctest
```

## Using minispdlog in Your Project

### Method 1: Find Package (after installation)

```cmake
# In your CMakeLists.txt
find_package(minispdlog REQUIRED)
target_link_libraries(your_target minispdlog::minispdlog)
```

### Method 2: Add Subdirectory

```cmake
# Add minispdlog as subdirectory
add_subdirectory(path/to/minispdlog)
target_link_libraries(your_target minispdlog)
```

### Method 3: FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    minispdlog
    GIT_REPOSITORY https://github.com/your_username/minispdlog.git
    GIT_TAG main
)

FetchContent_MakeAvailable(minispdlog)
target_link_libraries(your_target minispdlog)
```

## Cross-Platform Notes

### Windows
- Uses Windows threading APIs (CRITICAL_SECTION, CONDITION_VARIABLE)
- No external dependencies required
- Compatible with MSVC, MinGW, and Clang

### POSIX (Linux, macOS, Unix)
- Uses pthread for threading
- Requires pthread library (automatically linked by CMake)
- Compatible with GCC, Clang

## Directory Structure After Build

```
build/
├── example                 # Example executable
├── test                   # Test executable
├── CMakeCache.txt
├── CMakeFiles/
└── ...

# After installation:
/usr/local/
├── include/
│   └── minispdlog.h       # Header file
└── lib/
    └── cmake/
        └── minispdlog/    # CMake config files
```

## Troubleshooting

### Common Issues

1. **pthread not found on Linux/macOS**
   ```bash
   # Install development packages
   # Ubuntu/Debian:
   sudo apt-get install build-essential
   # CentOS/RHEL:
   sudo yum groupinstall "Development Tools"
   ```

2. **Windows compilation errors**
   - Ensure you're using a recent version of MinGW or Visual Studio
   - Try using the appropriate generator: `-G "Visual Studio 16 2019"`

3. **Permission denied during installation**
   ```bash
   # Use sudo on Linux/macOS
   sudo cmake --install . --prefix /usr/local
   
   # Or install to user directory
   cmake --install . --prefix ~/.local
   ```

### Build Types

- **Debug**: `-DCMAKE_BUILD_TYPE=Debug` (default for single-config generators)
- **Release**: `-DCMAKE_BUILD_TYPE=Release`
- **RelWithDebInfo**: `-DCMAKE_BUILD_TYPE=RelWithDebInfo`
- **MinSizeRel**: `-DCMAKE_BUILD_TYPE=MinSizeRel`

### Example Integration

```cmake
cmake_minimum_required(VERSION 3.10)
project(my_project C)

# Find minispdlog
find_package(minispdlog REQUIRED)

# Create your executable
add_executable(my_app main.c)

# Link with minispdlog
target_link_libraries(my_app minispdlog::minispdlog)
```

```c
// main.c
#include "minispdlog.h"

int main() {
    logger_init("my_app.log", LOG_LEVEL_INFO);
    
    logger_info("Application started");
    logger_debug("This won't appear (level is INFO)");
    logger_warn("Warning message");
    logger_error("Error message");
    
    logger_cleanup();
    return 0;
}
```
