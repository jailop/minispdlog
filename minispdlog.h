/**
 * minisdplog - A minimal spdlog-like header-only logging library
 *
 * This library provides a simple and efficient logging mechanism with support
 * for multiple log levels, asynchronous logging, and formatted messages.  It
 * is designed to be easy to use and integrate into existing C++ projects.
 *
 * License: MIT
 *
 * (c) 2025 Jaime Lopez <jailop AT protonmail DOT com>
 */

#ifndef _MINISDPLOG_H
#define _MINISDPLOG_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

namespace MiniLogger {

enum class LogLevel {
  DEBUG = 0,
  INFO,
  WARN,
  ERROR,
  CRITICAL,
};

class Logger {

public:
  explicit Logger(const std::string &filename,
                  LogLevel min_level = LogLevel::DEBUG, bool async_mode = false)
      : min_level_(min_level), async_mode_(async_mode), stop_thread_(false) {
    log_file_.open(filename, std::ios::app);
    if (!log_file_) {
      throw std::runtime_error("Failed to open log file");
    }
    if (async_mode_) {
      worker_thread_ = std::thread(&Logger::worker_function, this);
    }
  }

  ~Logger() {
    if (async_mode_) {
      stop_thread_ = true;
      cv_.notify_all();
      if (worker_thread_.joinable()) {
        worker_thread_.join();
      }
    }

    if (log_file_.is_open()) {
      log_file_.close();
    }
  }

  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  inline void set_level(LogLevel level) { min_level_ = level; }

  inline void debug(const std::string &message) {
    write_log(LogLevel::DEBUG, message);
  }

  inline void info(const std::string &message) {
    write_log(LogLevel::INFO, message);
  }

  inline void warn(const std::string &message) {
    write_log(LogLevel::WARN, message);
  }

  inline void error(const std::string &message) {
    write_log(LogLevel::ERROR, message);
  }

  inline void critical(const std::string &message) {
    write_log(LogLevel::CRITICAL, message);
  }

  template <typename... Args>
  inline void log(LogLevel level, const std::string &format, Args... args) {
    if (level < min_level_)
      return;
    std::ostringstream
        ss; // Changed from std::stringstream to std::ostringstream
    format_message(ss, format, args...);
    write_log(level, ss.str());
  }

  template <typename... Args>
  void debug(const std::string &format, Args... args) {
    log(LogLevel::DEBUG, format, args...);
  }

  template <typename... Args>
  void info(const std::string &format, Args... args) {
    log(LogLevel::INFO, format, args...);
  }

  template <typename... Args>
  void warn(const std::string &format, Args... args) {
    log(LogLevel::WARN, format, args...);
  }

  template <typename... Args>
  void error(const std::string &format, Args... args) {
    log(LogLevel::ERROR, format, args...);
  }

  template <typename... Args>
  void critical(const std::string &format, Args... args) {
    log(LogLevel::CRITICAL, format, args...);
  }

private:
  std::ofstream log_file_;
  std::mutex mutex_;
  LogLevel min_level_;
  bool async_mode_;

  // Async members
  std::queue<std::string> log_queue_;
  std::condition_variable cv_;
  std::thread worker_thread_;
  std::atomic<bool> stop_thread_;
  std::mutex queue_mutex_;

  inline std::string level_to_string(LogLevel level) {
    switch (level) {
    case LogLevel::DEBUG:
      return "DEBUG";
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::WARN:
      return "WARN";
    case LogLevel::ERROR:
      return "ERROR";
    case LogLevel::CRITICAL:
      return "CRITICAL";
    default:
      return "UNKNOWN";
    }
  }

  inline std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                  now.time_since_epoch()) %
              1000000;
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << '.'
       << std::setfill('0') << std::setw(6) << us.count();
    return ss.str();
  }

  void worker_function() {
    while (!stop_thread_ || !log_queue_.empty()) {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      cv_.wait(lock, [this] { return !log_queue_.empty() || stop_thread_; });

      while (!log_queue_.empty()) {
        std::string log_message = log_queue_.front();
        log_queue_.pop();
        lock.unlock();

        std::lock_guard<std::mutex> file_lock(mutex_);
        log_file_ << log_message << std::endl << std::flush;
        lock.lock();
      }
    }
  }

  void write_log(LogLevel level, const std::string &message) {
    if (level < min_level_)
      return;

    std::string log_message =
        get_timestamp() + " [" + level_to_string(level) + "] [Thread:" +
        std::to_string(
            std::hash<std::thread::id>{}(std::this_thread::get_id()) % 10000) +
        "] " + message;

    if (async_mode_) {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      log_queue_.push(log_message);
      cv_.notify_one();
    } else {
      std::lock_guard<std::mutex> file_lock(mutex_);
      log_file_ << log_message << std::endl << std::flush;
    }
  }

  template <typename T>
  void format_message(std::ostringstream &ss, const std::string &format,
                      T &&value) {
    size_t pos = format.find("{}");
    if (pos != std::string::npos) {
      ss << format.substr(0, pos) << value << format.substr(pos + 2);
    } else {
      ss << format;
    }
  }

  template <typename T, typename... Args>
  void format_message(std::ostringstream &ss, const std::string &format,
                      T &&value, Args &&...args) {
    size_t pos = format.find("{}");
    if (pos != std::string::npos) {
      ss << format.substr(0, pos) << value;
      format_message(ss, format.substr(pos + 2), std::forward<Args>(args)...);
    } else {
      ss << format;
    }
  }
};

class LoggerManager {
public:
  static void initialize(const std::string &filename,
                         LogLevel min_level = LogLevel::DEBUG,
                         bool async_mode = false) {
    auto &inst = get_instance();
    std::lock_guard<std::mutex> lock(inst.mutex);
    inst.logger = std::make_unique<Logger>(filename, min_level, async_mode);
    inst.initialized = true;
  }

  static Logger &get() {
    auto &inst = get_instance();
    if (!inst.initialized || !inst.logger) {
      throw std::runtime_error(
          "Logger not initialized. Call initialize() first.");
    }
    return *inst.logger;
  }

  static void shutdown() {
    auto &inst = get_instance();
    std::lock_guard<std::mutex> lock(inst.mutex);
    inst.logger.reset();
    inst.initialized = false;
  }

private:
  struct LoggerInstance {
    std::unique_ptr<Logger> logger;
    std::mutex mutex;
    bool initialized = false;

    ~LoggerInstance() {
      if (initialized && logger) {
        logger.reset();
      }
    }
  };

  static LoggerInstance &get_instance() {
    static LoggerInstance instance;
    return instance;
  }
};

} // namespace MiniLogger

#define SLOG_DEBUG(msg) MiniLogger::LoggerManager::get().debug(msg)
#define SLOG_INFO(msg) MiniLogger::LoggerManager::get().info(msg)
#define SLOG_WARN(msg) MiniLogger::LoggerManager::get().warn(msg)
#define SLOG_ERROR(msg) MiniLogger::LoggerManager::get().error(msg)
#define SLOG_CRITICAL(msg) MiniLogger::LoggerManager::get().critical(msg)

#define SLOG_DEBUG_F(fmt, ...)                                                 \
  MiniLogger::LoggerManager::get().debug(fmt, __VA_ARGS__)
#define SLOG_INFO_F(fmt, ...)                                                  \
  MiniLogger::LoggerManager::get().info(fmt, __VA_ARGS__)
#define SLOG_WARN_F(fmt, ...)                                                  \
  MiniLogger::LoggerManager::get().warn(fmt, __VA_ARGS__)
#define SLOG_ERROR_F(fmt, ...)                                                 \
  MiniLogger::LoggerManager::get().error(fmt, __VA_ARGS__)
#define SLOG_CRITICAL_F(fmt, ...)                                              \
  MiniLogger::LoggerManager::get().critical(fmt, __VA_ARGS__)

#endif // _MINISDPLOG_H
