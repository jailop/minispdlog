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
    #define _LOGGER_WINDOWS
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #define MINISPDLOG_POSIX
#endif

/* Platform-specific includes */
#ifdef _LOGGER_WINDOWS
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
#define _LOGGER_STDERR 2
#define _LOGGER_BUFFER_SIZE 8192
#define _LOGGER_MAX_LOG_ENTRY 1024

/* Configuration constants - centralized for easy maintenance */
#define LOGGER_TIMESTAMP_SIZE 128
#define LOGGER_SLEEP_MS_WIN 1
#define LOGGER_SLEEP_US_POSIX 1000

/**
 * To prevent the compiler warning that
 * a function is defined but has not being
 * used.
 */
#ifdef _MSC_VER
    #define _LOGGER_UNUSED
#else
    #define _LOGGER_UNUSED __attribute__((unused))
#endif

/* Log levels */
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_CRITICAL
} LogLevel;

#ifdef _LOGGER_WINDOWS
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

typedef struct {
    char buffer[_LOGGER_BUFFER_SIZE];
    int head;
    int tail;
    int count;
    logger_mutex_t mutex;
    logger_cond_t cond;
    int running;
} LoggerCircularBuffer;

typedef struct {
    int file;
    LogLevel min_level;
    int async_mode;
    LoggerCircularBuffer circ_buf;
    logger_thread_t writer_thread;
    logger_mutex_t sync_mutex;  /* Mutex for synchronous logging */
} Logger;

/* Global logger instance - initialized at runtime */
static Logger logger;
static int logger_initialized = 0;
static char logger_timestamp[LOGGER_TIMESTAMP_SIZE];


/* Cross-platform threading operations */
#ifdef _LOGGER_WINDOWS

static int _LOGGER_UNUSED logger_mutex_init(logger_mutex_t* mutex) {
    InitializeCriticalSection(&mutex->mutex);
    InitializeConditionVariable(&mutex->cond);
    mutex->initialized = 1;
    return 0;
}

static int _LOGGER_UNUSED logger_mutex_destroy(logger_mutex_t* mutex) {
    if (mutex->initialized) {
        DeleteCriticalSection(&mutex->mutex);
        mutex->initialized = 0;
    }
    return 0;
}

static inline int _LOGGER_UNUSED logger_mutex_lock(logger_mutex_t* mutex) {
    EnterCriticalSection(&mutex->mutex);
    return 0;
}

static inline int _LOGGER_UNUSED logger_mutex_unlock(logger_mutex_t* mutex) {
    LeaveCriticalSection(&mutex->mutex);
    return 0;
}

static inline int _LOGGER_UNUSED logger_cond_init(logger_cond_t* cond) {
    InitializeConditionVariable(&cond->cond);
    cond->initialized = 1;
    return 0;
}

static inline int _LOGGER_UNUSED logger_cond_destroy(logger_cond_t* cond) {
    cond->initialized = 0;
    return 0;
}

static inline int _LOGGER_UNUSED logger_cond_wait(logger_cond_t* cond, logger_mutex_t* mutex) {
    SleepConditionVariableCS(&cond->cond, &mutex->mutex, INFINITE);
    return 0;
}

static inline int _LOGGER_UNUSED logger_cond_signal(logger_cond_t* cond) {
    WakeConditionVariable(&cond->cond);
    return 0;
}

static int _LOGGER_UNUSED logger_thread_create(logger_thread_t* thread, logger_thread_func_t func, void* arg) {
    *thread = CreateThread(NULL, 0, func, arg, 0, NULL);
    return (*thread == NULL) ? -1 : 0;
}

static int _LOGGER_UNUSED logger_thread_join(logger_thread_t thread) {
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

static inline int _LOGGER_UNUSED logger_file_open(const char* filename) {
    return _open(filename, _O_WRONLY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE);
}

static inline int _LOGGER_UNUSED logger_file_write(int fd, const void* data, int len) {
#ifdef _LOGGER_WINDOWS
    return _write(fd, data, len);
#else
    return write(fd, data, len);
#endif
}

static inline int _LOGGER_UNUSED logger_file_close(int fd) {
    return _close(fd);
}

static void _LOGGER_UNUSED logger_get_time(struct tm** tm_ptr, long* microseconds) {
    struct _timeb tb;
    _ftime(&tb);
    *tm_ptr = localtime(&tb.time);
    *microseconds = tb.millitm * 1000;  /* Convert milliseconds to microseconds */
}

#endif /* _LOGGER_WINDOWS */

#ifdef MINISPDLOG_POSIX

static inline int _LOGGER_UNUSED logger_mutex_init(logger_mutex_t* mutex) {
    return pthread_mutex_init(mutex, NULL);
}

static inline int _LOGGER_UNUSED logger_mutex_destroy(logger_mutex_t* mutex) {
    return pthread_mutex_destroy(mutex);
}

static inline int _LOGGER_UNUSED logger_mutex_lock(logger_mutex_t* mutex) {
    return pthread_mutex_lock(mutex);
}

static inline int _LOGGER_UNUSED logger_mutex_unlock(logger_mutex_t* mutex) {
    return pthread_mutex_unlock(mutex);
}

static inline int _LOGGER_UNUSED logger_cond_init(logger_cond_t* cond) {
    return pthread_cond_init(cond, NULL);
}

static inline int _LOGGER_UNUSED logger_cond_destroy(logger_cond_t* cond) {
    return pthread_cond_destroy(cond);
}

static inline int _LOGGER_UNUSED logger_cond_wait(logger_cond_t* cond, logger_mutex_t* mutex) {
    return pthread_cond_wait(cond, mutex);
}

static inline int _LOGGER_UNUSED logger_cond_signal(logger_cond_t* cond) {
    return pthread_cond_signal(cond);
}

static inline int _LOGGER_UNUSED logger_thread_create(logger_thread_t* thread, logger_thread_func_t func, void* arg) {
    return pthread_create(thread, NULL, func, arg);
}

static inline int _LOGGER_UNUSED logger_thread_join(logger_thread_t thread) {
    return pthread_join(thread, NULL);
}

static inline int _LOGGER_UNUSED logger_file_open(const char* filename) {
    return open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
}

static inline int _LOGGER_UNUSED logger_file_write(int fd, const void* data, int len) {
    return write(fd, data, len);
}

static inline int _LOGGER_UNUSED logger_file_close(int fd) {
    return close(fd);
}

static void _LOGGER_UNUSED logger_get_time(struct tm** tm_ptr, long* microseconds) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *tm_ptr = localtime(&tv.tv_sec);
    *microseconds = tv.tv_usec;
}

#endif /* MINISPDLOG_POSIX */

static LOGGER_THREAD_RETURN _LOGGER_UNUSED writer_thread_func(void *arg) {
    char temp_buffer[_LOGGER_MAX_LOG_ENTRY];
    
    while (logger.circ_buf.running) {
        logger_mutex_lock(&logger.circ_buf.mutex);
        
        while (logger.circ_buf.count == 0 && logger.circ_buf.running) {
            logger_cond_wait(&logger.circ_buf.cond, &logger.circ_buf.mutex);
        }
        
        if (!logger.circ_buf.running) {
            logger_mutex_unlock(&logger.circ_buf.mutex);
            break;
        }
        
        /* Extract data from circular buffer */
        int bytes_to_write = 0;
        while (logger.circ_buf.count > 0 && bytes_to_write < _LOGGER_MAX_LOG_ENTRY - 1) {
            temp_buffer[bytes_to_write++] = logger.circ_buf.buffer[logger.circ_buf.tail];
            logger.circ_buf.tail = (logger.circ_buf.tail + 1) % _LOGGER_BUFFER_SIZE;
            logger.circ_buf.count--;
            
            if (temp_buffer[bytes_to_write - 1] == '\n') break;
        }
        
        logger_mutex_unlock(&logger.circ_buf.mutex);
        
        if (bytes_to_write > 0) {
            logger_file_write(logger.file, temp_buffer, bytes_to_write);
        }
    }
    
#ifdef _LOGGER_WINDOWS
    return 0;
#else
    return NULL;
#endif
}

/* Write data to circular buffer */
static int _LOGGER_UNUSED buffer_write(LoggerCircularBuffer* buf, const char* data, int len) {
    logger_mutex_lock(&buf->mutex);
    
    int written = 0;
    for (int i = 0; i < len && buf->count < _LOGGER_BUFFER_SIZE; i++) {
        buf->buffer[buf->head] = data[i];
        buf->head = (buf->head + 1) % _LOGGER_BUFFER_SIZE;
        buf->count++;
        written++;
    }
    
    if (written > 0) {
        logger_cond_signal(&buf->cond);
    }
    
    logger_mutex_unlock(&buf->mutex);
    return written;
}

/* Initialize logger defaults - separate concern from mode changes */
static void _LOGGER_UNUSED logger_init_defaults(void) {
    if (logger_initialized) return;
    
    memset(&logger, 0, sizeof(Logger));
    logger.file = _LOGGER_STDERR;
    logger.min_level = LOG_DEBUG;
    logger.async_mode = 0;
    logger_mutex_init(&logger.sync_mutex);
    logger_initialized = 1;
}

/* Shutdown async mode - extracted for clarity */
static void _LOGGER_UNUSED logger_shutdown_async(void) {
    if (!logger.async_mode) return;
    
    logger_mutex_lock(&logger.circ_buf.mutex);
    while (logger.circ_buf.count > 0) {
        logger_mutex_unlock(&logger.circ_buf.mutex);
#ifdef _LOGGER_WINDOWS
        Sleep(LOGGER_SLEEP_MS_WIN);
#else
        usleep(LOGGER_SLEEP_US_POSIX);
#endif
        logger_mutex_lock(&logger.circ_buf.mutex);
    }
    
    logger.circ_buf.running = 0;
    logger_cond_signal(&logger.circ_buf.cond);
    logger_mutex_unlock(&logger.circ_buf.mutex);
    
    logger_thread_join(logger.writer_thread);
    
    logger_mutex_destroy(&logger.circ_buf.mutex);
    logger_cond_destroy(&logger.circ_buf.cond);
}

/* Setup file output - separated for better error handling */
static int _LOGGER_UNUSED logger_setup_file(const char* filename) {
    if (logger.file != _LOGGER_STDERR) {
        logger_file_close(logger.file);
    }
    
    if (filename == NULL) {
        logger.file = _LOGGER_STDERR;
        return 0;
    }
    
    logger.file = logger_file_open(filename);
    if (logger.file == -1) {
        perror("Failed to open log file");
        logger.file = _LOGGER_STDERR;
        return -1;
    }
    return 0;
}

/* Initialize async mode - extracted for clarity */
static int _LOGGER_UNUSED logger_init_async(void) {
    logger.circ_buf.head = 0;
    logger.circ_buf.tail = 0;
    logger.circ_buf.count = 0;
    logger.circ_buf.running = 1;
    
    logger_mutex_init(&logger.circ_buf.mutex);
    logger_cond_init(&logger.circ_buf.cond);
    
    return logger_thread_create(&logger.writer_thread, 
                               (logger_thread_func_t)writer_thread_func, 
                               &logger);
}

/* Main init function - now much simpler and easier to understand */
static void _LOGGER_UNUSED logger_init(const char* filename, LogLevel min_level, int async_mode) {
    logger_init_defaults();
    
    /* Handle async mode changes */
    if (logger.async_mode && !async_mode) {
        logger_shutdown_async();
    }
    
    /* Setup file output */
    logger_setup_file(filename);
    
    /* Update settings */
    logger.min_level = min_level;
    
    /* Initialize async mode if requested */
    if (async_mode && !logger.async_mode) {
        logger_init_async();
    }
    
    /* Set async mode after potential initialization */
    logger.async_mode = async_mode;
}

/* Deinit function - simplified using helper functions */
static void _LOGGER_UNUSED logger_deinit(void) {
    if (logger.async_mode) {
        logger_shutdown_async();
    }
    
    if (logger.file > _LOGGER_STDERR) {
        logger_file_close(logger.file);
        logger.file = _LOGGER_STDERR;
    }
    
    /* Reset async mode flag */
    logger.async_mode = 0;
}

/* Reset logger to initial state - useful for testing */
static void _LOGGER_UNUSED logger_reset(void) {
    if (logger_initialized) {
        logger_deinit();
        logger_mutex_destroy(&logger.sync_mutex);
        logger_initialized = 0;
        memset(&logger, 0, sizeof(Logger));
    }
}

static inline void _LOGGER_UNUSED logger_set_min_level(LogLevel level) {
    logger.min_level = level;
}

/* Generate timestamp string */
static void _LOGGER_UNUSED logger_set_timestamp(void) {
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

static inline const char* _LOGGER_UNUSED logger_level_to_string(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARN: return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

static int _LOGGER_UNUSED logger_format_entry(char* buffer, size_t buffer_size, 
                                              LogLevel level, const char* message) {
    logger_set_timestamp();
    return snprintf(buffer, buffer_size, "%s [%s] %s\n", 
                   logger_timestamp, logger_level_to_string(level), message);
}

static void _LOGGER_UNUSED logger_write_log(LogLevel level, const char* message) {
    if (level < logger.min_level) {
        return;
    }
    
    /* Debug: Print what we're about to log */
    #ifdef DEBUG_LOGGER
    fprintf(stderr, "DEBUG: Writing log level %d (min: %d): %s\n", level, logger.min_level, message);
    #endif
    
    char log_entry[_LOGGER_MAX_LOG_ENTRY];
    int len = logger_format_entry(log_entry, sizeof(log_entry), level, message);
    
    if (len <= 0 || len >= _LOGGER_MAX_LOG_ENTRY) {
        return;
    }
    
    if (logger.async_mode) {
        buffer_write(&logger.circ_buf, log_entry, len);
    } else {
        logger_mutex_lock(&logger.sync_mutex);
        logger_file_write(logger.file, log_entry, len);
        logger_mutex_unlock(&logger.sync_mutex);
    }
}

/* Basic logging functions - generated via macro to reduce duplication */
#define DEFINE_LOGGER_FUNC(level_name, level_enum) \
    static inline void _LOGGER_UNUSED logger_##level_name(const char* message) { \
        logger_write_log(level_enum, message); \
    }

DEFINE_LOGGER_FUNC(debug, LOG_DEBUG)
DEFINE_LOGGER_FUNC(info, LOG_INFO)
DEFINE_LOGGER_FUNC(warn, LOG_WARN)
DEFINE_LOGGER_FUNC(error, LOG_ERROR)
DEFINE_LOGGER_FUNC(critical, LOG_CRITICAL)

static void _LOGGER_UNUSED logger_write_log_va(LogLevel level, const char* format, va_list args) {
    if (level < logger.min_level) {
        return;
    }
    char message[_LOGGER_MAX_LOG_ENTRY];
    vsnprintf(message, sizeof(message), format, args);
    logger_write_log(level, message);
}

static void _LOGGER_UNUSED logger_write_log_f(LogLevel level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logger_write_log_va(level, format, args);
    va_end(args);
}

/* Formatted logging functions - generated via macro to reduce duplication */
#define DEFINE_LOGGER_FUNC_F(level_name, level_enum) \
    static inline void _LOGGER_UNUSED logger_##level_name##_f(const char* format, ...) { \
        va_list args; \
        va_start(args, format); \
        logger_write_log_va(level_enum, format, args); \
        va_end(args); \
    }

DEFINE_LOGGER_FUNC_F(debug, LOG_DEBUG)
DEFINE_LOGGER_FUNC_F(info, LOG_INFO)
DEFINE_LOGGER_FUNC_F(warn, LOG_WARN)
DEFINE_LOGGER_FUNC_F(error, LOG_ERROR)
DEFINE_LOGGER_FUNC_F(critical, LOG_CRITICAL)

/* Clean up the macros to avoid polluting the namespace */
#undef DEFINE_LOGGER_FUNC
#undef DEFINE_LOGGER_FUNC_F

#endif /* _MINISPDLOG_H */
