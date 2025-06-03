#include <stdio.h>
#include "minispdlog.h"

int main() {
    printf("minispdlog Example\n");
    printf("==================\n");

    logger_init("example.log", DEBUG, 1);

    printf("\nLogging messages...\n");
    logger_info("Application started");
    logger_debug_f("Debug value: %d", 42);
    logger_warn("This is a warning message");
    logger_error_f("Failed to process file: %s", "data.txt");
    logger_critical("Critical system failure!");

    printf("Changing log level to ERROR...\n");
    logger_set_min_level(ERROR);
    logger_info("This won't appear in log");  // Below ERROR level
    logger_error("This will appear in log");

    logger_set_min_level(DEBUG);
    logger_info("Log level reset to DEBUG");

    logger_info_f("User %s has %d points, accuracy: %.2f%%", "alice", 1250, 98.5);
    logger_debug_f("Memory usage: %lu bytes", (unsigned long)1024*1024);
    logger_warn_f("Temperature warning: %d°C", 85);

    // Clean up
    logger_deinit();

    printf("✓ Logging completed. Check 'example.log' for output.\n");
    return 0;
}
