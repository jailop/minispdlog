#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include "minispdlog.h"

void test_crossplatform_sync_logging() {
    printf("Testing synchronous logging...\n");
    
    // Test logging to stderr
    logger_init(NULL, LOG_DEBUG, 0);
    logger_info("Sync test to stderr");
    logger_debug("Debug message");
    logger_warn("Warning message");
    logger_error("Error message");
    logger_critical("Critical message");
    logger_deinit();
    
    // Test logging to file
    const char* test_file = "test_sync.log";
    logger_init(test_file, LOG_INFO, 0);
    logger_debug("This should not appear (below LOG_INFO level)");
    logger_info("This should appear in file");
    logger_warn("Warning in file");
    logger_error_f("Error with number: %d", 42);
    logger_critical_f("Critical with string: %s", "test");
    logger_deinit();
    
    // Check if file was created and has content
    struct stat st;
    assert(stat(test_file, &st) == 0);
    assert(st.st_size > 0);
    
    printf("✓ Synchronous logging test passed\n");
}

void test_crossplatform_async_logging() {
    printf("Testing asynchronous logging...\n");
    
    const char* test_file = "test_async.log";
    logger_init(test_file, LOG_DEBUG, 1);
    
    // Generate multiple log entries quickly
    for (int i = 0; i < 50; i++) {
        logger_info_f("Async message %d", i);
        if (i % 10 == 0) {
            logger_warn_f("Warning at iteration %d", i);
        }
    }
    
    // Give some time for async writes
    struct timespec ts = {0, 100000000}; /* 100ms */
    nanosleep(&ts, NULL);
    
    logger_deinit();
    
    // Check if file was created and has content
    struct stat st;
    assert(stat(test_file, &st) == 0);
    assert(st.st_size > 0);
    
    printf("✓ Asynchronous logging test passed\n");
}

void test_crossplatform_log_levels() {
    printf("Testing log levels...\n");
    
    const char* test_file = "test_levels.log";
    
    // Test with LOG_WARN level - should only show LOG_WARN, LOG_ERROR, LOG_CRITICAL
    logger_init(test_file, LOG_WARN, 0);
    logger_debug("Should not appear");
    logger_info("Should not appear");
    logger_warn("Should appear");
    logger_error("Should appear");
    logger_critical("Should appear");
    logger_deinit();
    
    // Read file and count lines
    FILE* f = fopen(test_file, "r");
    assert(f != NULL);
    
    char line[256];
    int line_count = 0;
    while (fgets(line, sizeof(line), f)) {
        line_count++;
    }
    fclose(f);
    
    // Should have exactly 3 lines (LOG_WARN, ERROR, CRITICAL)
    assert(line_count == 3);
    
    printf("✓ Log levels test passed\n");
}

void test_crossplatform_formatted_messages() {
    printf("Testing formatted messages...\n");
    
    const char* test_file = "test_formatted.log";
    logger_init(test_file, LOG_DEBUG, 0);
    
    logger_debug_f("Debug: %d %s %.2f", 123, "test", 3.14);
    logger_info_f("Info: %c %x", 'A', 255);
    logger_warn_f("Warn: %ld", 1234567890L);
    logger_error_f("Error: %p", (void*)0x12345678);
    logger_critical_f("Critical: %% literal percent");
    
    logger_deinit();
    
    // Check if file was created and has content
    struct stat st;
    assert(stat(test_file, &st) == 0);
    assert(st.st_size > 0);
    
    printf("✓ Formatted messages test passed\n");
}

void test_crossplatform_min_level_change() {
    printf("Testing minimum level changes...\n");
    
    logger_init(NULL, LOG_DEBUG, 0);
    
    // Test initial level
    logger_info("This should appear");
    
    // Change to ERROR level
    logger_set_min_level(ERROR);
    logger_info("This should NOT appear");
    logger_error("This should appear");
    
    // Change back to LOG_DEBUG
    logger_set_min_level(LOG_DEBUG);
    logger_debug("This should appear again");
    
    logger_deinit();
    
    printf("✓ Minimum level change test passed\n");
}

void cleanup_test_files() {
    unlink("test_sync.log");
    unlink("test_async.log");
    unlink("test_levels.log");
    unlink("test_formatted.log");
}

int main() {
    printf("Running minispdlog tests...\n\n");
    
    test_crossplatform_sync_logging();
    test_crossplatform_async_logging();
    test_crossplatform_log_levels();
    test_crossplatform_formatted_messages();
    test_crossplatform_min_level_change();
    
    cleanup_test_files();
    
    printf("\n✓ All tests passed!\n");
    printf("Library successfully works on this platform.\n");
    return 0;
}
