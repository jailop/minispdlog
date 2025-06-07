#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "minispdlog.h"

void test_crossplatform_sync_logging(void) {
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

void test_crossplatform_async_logging(void) {
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

void test_crossplatform_log_levels(void) {
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

void test_crossplatform_formatted_messages(void) {
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

void test_crossplatform_min_level_change(void) {
    printf("Testing minimum level changes...\n");
    
    logger_init(NULL, LOG_DEBUG, 0);
    
    // Test initial level
    logger_info("This should appear");
    
    // Change to ERROR level
    logger_set_min_level(LOG_ERROR);
    logger_info("This should NOT appear");
    logger_error("This should appear");
    
    // Change back to LOG_DEBUG
    logger_set_min_level(LOG_DEBUG);
    logger_debug("This should appear again");
    
    logger_deinit();
    
    printf("✓ Minimum level change test passed\n");
}

typedef struct {
    int thread_id;
    int num_messages;
} thread_data_t;

void* thread_logging_func(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    
    for (int i = 0; i < data->num_messages; i++) {
        logger_info_f("Thread %d - Message %d", data->thread_id, i);
        logger_warn_f("Thread %d - Warning %d", data->thread_id, i);
        if (i % 5 == 0) {
            logger_error_f("Thread %d - Error at iteration %d", data->thread_id, i);
        }
        
        struct timespec ts = {0, 1000000}; /* 1ms */
        nanosleep(&ts, NULL);
    }
    
    return NULL;
}

void test_multithreaded_sync_logging(void) {
    printf("Testing multi-threaded synchronous logging...\n");
    
    const char* test_file = "test_multithreaded_sync.log";
    const int num_threads = 5;
    const int messages_per_thread = 10;
    
    logger_init(test_file, LOG_DEBUG, 0);
    
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].num_messages = messages_per_thread;
        
        int result = pthread_create(&threads[i], NULL, thread_logging_func, &thread_data[i]);
        assert(result == 0);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    logger_deinit();
    
    // Check if file was created and has expected content
    struct stat st;
    assert(stat(test_file, &st) == 0);
    assert(st.st_size > 0);
    
    // Count lines in the file
    FILE* f = fopen(test_file, "r");
    assert(f != NULL);
    
    char line[512];
    int line_count = 0;
    while (fgets(line, sizeof(line), f)) {
        line_count++;
    }
    fclose(f);
    
    // Each thread writes: messages_per_thread info + messages_per_thread warn + 2 error (at i=0 and i=5)
    int expected_lines = num_threads * (messages_per_thread * 2 + 2);
    assert(line_count == expected_lines);
    
    printf("✓ Multi-threaded synchronous logging test passed\n");
}

void test_multithreaded_async_logging(void) {
    printf("Testing multi-threaded asynchronous logging...\n");
    
    const char* test_file = "test_multithreaded_async.log";
    const int num_threads = 5;
    const int messages_per_thread = 10;
    
    logger_init(test_file, LOG_DEBUG, 1);
    
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];
    
    // Create threads for async test
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i + 100; // Different IDs to distinguish from sync test
        thread_data[i].num_messages = messages_per_thread;
        
        int result = pthread_create(&threads[i], NULL, thread_logging_func, &thread_data[i]);
        assert(result == 0);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Give time for async writes to complete
    struct timespec ts = {0, 200000000}; /* 200ms */
    nanosleep(&ts, NULL);
    
    logger_deinit();
    
    // Check async file
    struct stat st;
    assert(stat(test_file, &st) == 0);
    assert(st.st_size > 0);
    
    printf("✓ Multi-threaded asynchronous logging test passed\n");
}

void cleanup_test_files(void) {
    unlink("test_sync.log");
    unlink("test_async.log");
    unlink("test_levels.log");
    unlink("test_formatted.log");
    unlink("test_multithreaded_sync.log");
    unlink("test_multithreaded_async.log");
}

int main(void) {
    printf("Running minispdlog tests...\n\n");
    
    test_crossplatform_sync_logging();
    test_crossplatform_async_logging();
    test_crossplatform_log_levels();
    test_crossplatform_formatted_messages();
    test_crossplatform_min_level_change();
    test_multithreaded_sync_logging();
    test_multithreaded_async_logging();
    
    cleanup_test_files();
    
    printf("\n✓ All tests passed!\n");
    printf("Library successfully works on this platform.\n");
    return 0;
}
