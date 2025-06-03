/**
 * minispdlog - A minimal spdlog-like header-only logging library for C99
 *
 * This library provides a simple and efficient cross-platform logging mechanism
 * with support for multiple log levels, asynchronous logging, and formatted
 * messages.  It is designed to be easy to use and integrate into existing C
 * projects.
 * 
 * Supports: Windows (Win32), POSIX (Linux, macOS, Unix-like systems)
 *
 * License: MIT
 *
 * (c) 2025 Jaime Lopez <https://github.com/jailop/minispdlog>
 */

#ifndef _MINISPDLOG_H
#define _MINISPDLOG_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Platform detection */
#ifdef _WIN32
    #define MINISPDLOG_WINDOWS
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #define MINISPDLOG_POSIX
#endif

/* Platform-specific includes */
#ifdef MINISPDLOG_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/timeb.h>
    #include <process.h>
#endif

#ifdef MINISPDLOG_POSIX
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/time.h>
    #include <pthread.h>
#endif

/* Constants */
#define LOGGER_STDERR 2
#define BUFFER_SIZE 8192
#define MAX_LOG_ENTRY 1024

/* Cross-platform attribute handling */
#ifdef _MSC_VER
    #define MINISPDLOG_UNUSED
#else
    #define MINISPDLOG_UNUSED __attribute__((unused))
#endif

/* Log levels */
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_CRITICAL
} LogLevel;

/* Cross-platform threading abstractions */
#ifdef MINISPDLOG_WINDOWS
    typedef struct {
        CRITICAL_SECTION mutex;
        CONDITION_VARIABLE cond;
        int initialized;
    } logger_mutex_t;
    
    typedef struct {
        CONDITION_VARIABLE cond;
        int initialized;
    } logger_cond_t;
    
    typedef HANDLE logger_thread_t;
    typedef DWORD (WINAPI *logger_thread_func_t)(LPVOID);
    
    #define LOGGER_THREAD_RETURN DWORD WINAPI
    
#endif

#ifdef MINISPDLOG_POSIX
    typedef pthread_mutex_t logger_mutex_t;
    typedef pthread_cond_t logger_cond_t;
    typedef pthread_t logger_thread_t;
    typedef void* (*logger_thread_func_t)(void*);
    
    #define LOGGER_THREAD_RETURN void*
#endif

/* Circular buffer structure */
typedef struct {
    char buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    logger_mutex_t mutex;
    logger_cond_t cond;
    int running;
} CircularBuffer;

/* Logger structure */
typedef struct {
    int file;
    LogLevel min_level;
    int async_mode;
    CircularBuffer circ_buf;
    logger_thread_t writer_thread;
} Logger;

/* Cross-platform threading operations */
#ifdef MINISPDLOG_WINDOWS

static int MINISPDLOG_UNUSED logger_mutex_init(logger_mutex_t* mutex) {
    InitializeCriticalSection(&mutex->mutex);
    InitializeConditionVariable(&mutex->cond);
    mutex->initialized = 1;
    return 0;
}

static int MINISPDLOG_UNUSED logger_mutex_destroy(logger_mutex_t* mutex) {
    if (mutex->initialized) {
        DeleteCriticalSection(&mutex->mutex);
        mutex->initialized = 0;
    }
    return 0;
}

static inline int MINISPDLOG_UNUSED logger_mutex_lock(logger_mutex_t* mutex) {
    EnterCriticalSection(&mutex->mutex);
    return 0;
}

static inline int MINISPDLOG_UNUSED logger_mutex_unlock(logger_mutex_t* mutex) {
    LeaveCriticalSection(&mutex->mutex);
    return 0;
}

static inline int MINISPDLOG_UNUSED logger_cond_init(logger_cond_t* cond) {
    InitializeConditionVariable(&cond->cond);
    cond->initialized = 1;
    return 0;
}

static inline int MINISPDLOG_UNUSED logger_cond_destroy(logger_cond_t* cond) {
    cond->initialized = 0;
    return 0;
}

static inline int MINISPDLOG_UNUSED logger_cond_wait(logger_cond_t* cond, logger_mutex_t* mutex) {
    SleepConditionVariableCS(&cond->cond, &mutex->mutex, INFINITE);
    return 0;
}

static inline int MINISPDLOG_UNUSED logger_cond_signal(logger_cond_t* cond) {
    WakeConditionVariable(&cond->cond);
    return 0;
}

static int MINISPDLOG_UNUSED logger_thread_create(logger_thread_t* thread, logger_thread_func_t func, void* arg) {
    *thread = CreateThread(NULL, 0, func, arg, 0, NULL);
    return (*thread == NULL) ? -1 : 0;
}

static int MINISPDLOG_UNUSED logger_thread_join(logger_thread_t thread) {
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

static inline int MINISPDLOG_UNUSED logger_file_open(const char* filename) {
    return _open(filename, _O_WRONLY | _O_CREAT | _O_APPEND, _S_IREAD | _S_IWRITE);
}

static inline int MINISPDLOG_UNUSED logger_file_write(int fd, const void* data, int len) {
    return _write(fd, data, len);
}

static inline int MINISPDLOG_UNUSED logger_file_close(int fd) {
    return _close(fd);
}

static void MINISPDLOG_UNUSED logger_get_time(struct tm** tm_ptr, long* microseconds) {
    struct _timeb tb;
    _ftime(&tb);
    *tm_ptr = localtime(&tb.time);
    *microseconds = tb.millitm * 1000;  /* Convert milliseconds to microseconds */
}

#endif /* MINISPDLOG_WINDOWS */

#ifdef MINISPDLOG_POSIX

static inline int MINISPDLOG_UNUSED logger_mutex_init(logger_mutex_t* mutex) {
    return pthread_mutex_init(mutex, NULL);
}

static inline int MINISPDLOG_UNUSED logger_mutex_destroy(logger_mutex_t* mutex) {
    return pthread_mutex_destroy(mutex);
}

static inline int MINISPDLOG_UNUSED logger_mutex_lock(logger_mutex_t* mutex) {
    return pthread_mutex_lock(mutex);
}

static inline int MINISPDLOG_UNUSED logger_mutex_unlock(logger_mutex_t* mutex) {
    return pthread_mutex_unlock(mutex);
}

static inline int MINISPDLOG_UNUSED logger_cond_init(logger_cond_t* cond) {
    return pthread_cond_init(cond, NULL);
}

static inline int MINISPDLOG_UNUSED logger_cond_destroy(logger_cond_t* cond) {
    return pthread_cond_destroy(cond);
}

static inline int MINISPDLOG_UNUSED logger_cond_wait(logger_cond_t* cond, logger_mutex_t* mutex) {
    return pthread_cond_wait(cond, mutex);
}

static inline int MINISPDLOG_UNUSED logger_cond_signal(logger_cond_t* cond) {
    return pthread_cond_signal(cond);
}

static inline int MINISPDLOG_UNUSED logger_thread_create(logger_thread_t* thread, logger_thread_func_t func, void* arg) {
    return pthread_create(thread, NULL, func, arg);
}

static inline int MINISPDLOG_UNUSED logger_thread_join(logger_thread_t thread) {
    return pthread_join(thread, NULL);
}

static inline int MINISPDLOG_UNUSED logger_file_open(const char* filename) {
    return open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
}

static inline int MINISPDLOG_UNUSED logger_file_write(int fd, const void* data, int len) {
    return write(fd, data, len);
}

static inline int MINISPDLOG_UNUSED logger_file_close(int fd) {
    return close(fd);
}

static void MINISPDLOG_UNUSED logger_get_time(struct tm** tm_ptr, long* microseconds) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *tm_ptr = localtime(&tv.tv_sec);
    *microseconds = tv.tv_usec;
}

#endif /* MINISPDLOG_POSIX */

/* Global logger instance - initialized at runtime */
static Logger logger;
static int logger_initialized = 0;
static char logger_timestamp[128];

/* Thread function for asynchronous logging */
static LOGGER_THREAD_RETURN MINISPDLOG_UNUSED writer_thread_func(void* arg) {
    Logger* log = (Logger*)arg;
    char temp_buffer[MAX_LOG_ENTRY];
    
    while (log->circ_buf.running) {
        logger_mutex_lock(&log->circ_buf.mutex);
        
        while (log->circ_buf.count == 0 && log->circ_buf.running) {
            logger_cond_wait(&log->circ_buf.cond, &log->circ_buf.mutex);
        }
        
        if (!log->circ_buf.running) {
            logger_mutex_unlock(&log->circ_buf.mutex);
            break;
        }
        
        /* Extract data from circular buffer */
        int bytes_to_write = 0;
        while (log->circ_buf.count > 0 && bytes_to_write < MAX_LOG_ENTRY - 1) {
            temp_buffer[bytes_to_write++] = log->circ_buf.buffer[log->circ_buf.tail];
            log->circ_buf.tail = (log->circ_buf.tail + 1) % BUFFER_SIZE;
            log->circ_buf.count--;
            
            if (temp_buffer[bytes_to_write - 1] == '\n') break;
        }
        
        logger_mutex_unlock(&log->circ_buf.mutex);
        
        if (bytes_to_write > 0) {
            logger_file_write(log->file, temp_buffer, bytes_to_write);
        }
    }
    
#ifdef MINISPDLOG_WINDOWS
    return 0;
#else
    return NULL;
#endif
}

/* Write data to circular buffer */
static int MINISPDLOG_UNUSED buffer_write(CircularBuffer* buf, const char* data, int len) {
    logger_mutex_lock(&buf->mutex);
    
    int written = 0;
    for (int i = 0; i < len && buf->count < BUFFER_SIZE; i++) {
        buf->buffer[buf->head] = data[i];
        buf->head = (buf->head + 1) % BUFFER_SIZE;
        buf->count++;
        written++;
    }
    
    if (written > 0) {
        logger_cond_signal(&buf->cond);
    }
    
    logger_mutex_unlock(&buf->mutex);
    return written;
}

/* Initialize the logger */
static void MINISPDLOG_UNUSED logger_init(const char* filename, LogLevel min_level, int async_mode) {
    /* First-time initialization */
    if (!logger_initialized) {
        logger.file = LOGGER_STDERR;
        logger.min_level = LOG_DEBUG;
        logger.async_mode = 0;
        memset(&logger.circ_buf, 0, sizeof(CircularBuffer));
        logger.writer_thread = 0;
        logger_initialized = 1;
    }
    
    /* Close existing file if open */
    if (logger.file > LOGGER_STDERR) {
        logger_file_close(logger.file);
    }
    
    /* Open new file or use stderr */
    if (filename == NULL) {
        logger.file = LOGGER_STDERR;
    } else {
        logger.file = logger_file_open(filename);
        if (logger.file == -1) {
            perror("Failed to open log file");
            logger.file = LOGGER_STDERR;
        }
    }
    
    logger.min_level = min_level;
    logger.async_mode = async_mode;
    
    if (async_mode) {
        /* Initialize circular buffer */
        logger.circ_buf.head = 0;
        logger.circ_buf.tail = 0;
        logger.circ_buf.count = 0;
        logger.circ_buf.running = 1;
        
        /* Initialize synchronization primitives */
        logger_mutex_init(&logger.circ_buf.mutex);
        logger_cond_init(&logger.circ_buf.cond);
        
        /* Create worker thread */
        logger_thread_create(&logger.writer_thread, 
                           (logger_thread_func_t)writer_thread_func, 
                           &logger);
    }
}

/* Cleanup the logger */
static void MINISPDLOG_UNUSED logger_deinit() {
    if (logger.async_mode) {
        /* Signal thread to stop */
        logger_mutex_lock(&logger.circ_buf.mutex);
        logger.circ_buf.running = 0;
        logger_cond_signal(&logger.circ_buf.cond);
        logger_mutex_unlock(&logger.circ_buf.mutex);
        
        /* Wait for thread to finish */
        logger_thread_join(logger.writer_thread);
        
        /* Cleanup synchronization primitives */
        logger_mutex_destroy(&logger.circ_buf.mutex);
        logger_cond_destroy(&logger.circ_buf.cond);
    }
    
    /* Close file if not stderr */
    if (logger.file > LOGGER_STDERR) {
        logger_file_close(logger.file);
        logger.file = LOGGER_STDERR;
    }
}

/* Set minimum log level */
static inline void MINISPDLOG_UNUSED logger_set_min_level(LogLevel level) {
    logger.min_level = level;
}

/* Generate timestamp string */
static void MINISPDLOG_UNUSED logger_set_timestamp() {
    struct tm *now_tm;
    long microseconds;
    
    logger_get_time(&now_tm, &microseconds);
    
    if (now_tm) {
        int base_len = strftime(logger_timestamp, sizeof(logger_timestamp), 
                               "%Y-%m-%d %H:%M:%S", now_tm);
        if (base_len > 0 && base_len < (int)sizeof(logger_timestamp) - 8) {
            snprintf(logger_timestamp + base_len, 
                    sizeof(logger_timestamp) - base_len, 
                    ".%06ld", microseconds);
        }
    } else {
        strcpy(logger_timestamp, "1970-01-01 00:00:00.000000");
    }
}

/* Convert log level to string */
static inline const char* MINISPDLOG_UNUSED logger_level_to_string(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARN: return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

/* Write log message */
static void MINISPDLOG_UNUSED logger_write_log(LogLevel level, const char* message) {
    if (level < logger.min_level) {
        return;
    }
    
    logger_set_timestamp();
    char log_entry[MAX_LOG_ENTRY];
    int len = snprintf(log_entry, sizeof(log_entry), "%s [%s] %s\n", 
                      logger_timestamp, logger_level_to_string(level), message);
    
    if (len > 0 && len < MAX_LOG_ENTRY) {
        if (logger.async_mode) {
            buffer_write(&logger.circ_buf, log_entry, len);
        } else {
            logger_file_write(logger.file, log_entry, len);
        }
    }
}

/* Basic logging functions */
static inline void MINISPDLOG_UNUSED logger_debug(const char* message) {
    logger_write_log(LOG_DEBUG, message);
}

static inline void MINISPDLOG_UNUSED logger_info(const char* message) {
    logger_write_log(LOG_INFO, message);
}

static inline void MINISPDLOG_UNUSED logger_warn(const char* message) {
    logger_write_log(LOG_WARN, message);
}

static inline void MINISPDLOG_UNUSED logger_error(const char* message) {
    logger_write_log(LOG_ERROR, message);
}

static inline void MINISPDLOG_UNUSED logger_critical(const char* message) {
    logger_write_log(LOG_CRITICAL, message);
}

static void MINISPDLOG_UNUSED logger_write_log_va(LogLevel level, const char* format, va_list args) {
    if (level < logger.min_level) {
        return;
    }
    char message[MAX_LOG_ENTRY];
    vsnprintf(message, sizeof(message), format, args);
    logger_write_log(level, message);
}

static void MINISPDLOG_UNUSED logger_write_log_f(LogLevel level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logger_write_log_va(level, format, args);
    va_end(args);
}

static inline void MINISPDLOG_UNUSED logger_debug_f(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logger_write_log_va(LOG_DEBUG, format, args);
    va_end(args);
}

static inline void MINISPDLOG_UNUSED logger_info_f(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logger_write_log_va(LOG_INFO, format, args);
    va_end(args);
}

static inline void MINISPDLOG_UNUSED logger_warn_f(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logger_write_log_va(LOG_WARN, format, args);
    va_end(args);
}

static inline void MINISPDLOG_UNUSED logger_error_f(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logger_write_log_va(LOG_ERROR, format, args);
    va_end(args);
}

static inline void MINISPDLOG_UNUSED logger_critical_f(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logger_write_log_va(LOG_CRITICAL, format, args);
    va_end(args);
}

#endif /* _MINISPDLOG_H */
