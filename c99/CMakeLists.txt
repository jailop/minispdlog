# CMakeLists.txt for minispdlog C99 Cross-Platform Library
cmake_minimum_required(VERSION 3.10)

# Project configuration
project(minispdlog 
    VERSION 1.0.0
    DESCRIPTION "A minimal spdlog-like header-only logging library for C99"
    LANGUAGES C
)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build configuration options
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build test programs" ON)
option(ENABLE_WARNINGS "Enable compiler warnings" ON)

# Platform detection and configuration
if(WIN32)
    set(PLATFORM_NAME "Windows")
    message(STATUS "Configuring for Windows platform")
elseif(UNIX)
    set(PLATFORM_NAME "POSIX")
    message(STATUS "Configuring for POSIX platform (Linux/macOS/Unix)")
else()
    set(PLATFORM_NAME "Unknown")
    message(WARNING "Unknown platform detected")
endif()

# Compiler-specific flags
if(ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(/W4 /WX)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

# Platform-specific compile definitions
if(WIN32)
    add_compile_definitions(_WIN32)
    # Ensure Windows threading support
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
elseif(UNIX)
    add_compile_definitions(_POSIX_C_SOURCE=200809L)
endif()

# Create interface library for header-only library
add_library(minispdlog INTERFACE)

# Set target properties
target_include_directories(minispdlog INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Platform-specific linking
if(UNIX AND NOT APPLE)
    # Linux needs pthread
    find_package(Threads REQUIRED)
    target_link_libraries(minispdlog INTERFACE Threads::Threads)
elseif(APPLE)
    # macOS needs pthread
    find_package(Threads REQUIRED)
    target_link_libraries(minispdlog INTERFACE Threads::Threads)
elseif(WIN32)
    # Windows uses built-in threading
    target_link_libraries(minispdlog INTERFACE)
endif()

# Build examples if requested
if(BUILD_EXAMPLES)
    add_executable(minispdlog_example example.c)
    target_link_libraries(minispdlog_example minispdlog)
    
    # Set output name
    set_target_properties(minispdlog_example PROPERTIES OUTPUT_NAME "example")
    
    message(STATUS "Building example program")
endif()

# Build tests if requested
if(BUILD_TESTS)
    add_executable(minispdlog_test test.c)
    target_link_libraries(minispdlog_test minispdlog)
    
    # Set output name
    set_target_properties(minispdlog_test PROPERTIES OUTPUT_NAME "test")
    
    # Enable testing
    enable_testing()
    add_test(NAME minispdlog_unit_tests COMMAND minispdlog_test)
    
    message(STATUS "Building test program")
endif()

# Installation configuration
include(GNUInstallDirs)

# Install header file
install(FILES minispdlog.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
)

# Install CMake targets
install(TARGETS minispdlog
    EXPORT minispdlogTargets
    COMPONENT Development
)

# Install CMake config files
install(EXPORT minispdlogTargets
    FILE minispdlogTargets.cmake
    NAMESPACE minispdlog::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/minispdlog
    COMPONENT Development
)

# Create and install config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/minispdlogConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/minispdlogConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/minispdlog
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/minispdlogConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/minispdlogConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/minispdlogConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/minispdlog
    COMPONENT Development
)

# Create package config template directory if it doesn't exist
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Print build summary
message(STATUS "")
message(STATUS "=== minispdlog Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Platform: ${PLATFORM_NAME}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "  BUILD_EXAMPLES: ${BUILD_EXAMPLES}")
message(STATUS "  BUILD_TESTS: ${BUILD_TESTS}")
message(STATUS "  ENABLE_WARNINGS: ${ENABLE_WARNINGS}")
message(STATUS "")
message(STATUS "Install Directories:")
message(STATUS "  Headers: ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
message(STATUS "  CMake: ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/minispdlog")
message(STATUS "========================================")
message(STATUS "")

# Custom targets for convenience
add_custom_target(run-example
    COMMAND $<TARGET_FILE:minispdlog_example>
    DEPENDS minispdlog_example
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running example program"
)

add_custom_target(run-tests
    COMMAND $<TARGET_FILE:minispdlog_test>
    DEPENDS minispdlog_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running test program"
)

# Clean target for log files
add_custom_target(clean-logs
    COMMAND ${CMAKE_COMMAND} -E remove -f "*.log"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Cleaning log files"
)
