#include "minispdlog.h"

int main() {
    MiniLogger::LoggerManager::initialize("log.txt", MiniLogger::LogLevel::DEBUG, false);
    SLOG_INFO("Application started");
    SLOG_DEBUG("This is a debug message");
    SLOG_WARN("This is a warning message");
    SLOG_ERROR("This is an error message");
    SLOG_INFO_F("User {} logger in with ID {}", "Alice", 12345);
    auto& logger = MiniLogger::LoggerManager::get();
    logger.warn("Direct warning message");
}
