/************************************************************************
* @Copyright: 2023-2024
* @FileName:
* @Description: Open source mediasoup C++ controller library
* @Version: 1.0.0
* @Author: Jackie Ou
* @CreateTime: 2023-10-30
*************************************************************************/

#pragma once

#include <stdio.h>
#include <cstddef>
#include <mutex>

namespace srv {

enum class SrvLogLevel : int
{
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERR   = 3,  // 'ERROR' is a Windows macro, avoid the name
    NONE  = 4
};

// Global log file handle (nullptr means file logging disabled)
extern FILE*        g_srvLogFile;
extern SrvLogLevel  g_srvLogLevel;
extern std::mutex   g_srvLogMutex;
// Maximum size in bytes before rotation (default 50 MB)
extern std::size_t  g_srvLogMaxBytes;
// Number of rotated backup files to keep (default 3, i.e. .1 .2 .3)
extern int          g_srvLogBackupCount;

/**
 * Initialize file logging.
 * @param path          log file path; if null/empty, file logging is disabled.
 * @param level         minimum level that will be written to the file.
 * @param maxBytes      rotate when current file exceeds this size (0 = no rotation).
 * @param backupCount   number of rotated backup files to keep (0 = keep only current).
 * @return true on success, false on failure (stderr output is preserved either way).
 */
bool srv_logger_init(const char* path, SrvLogLevel level,
                     std::size_t maxBytes = 50 * 1024 * 1024,
                     int backupCount = 3);

/** Flush and close the log file. Safe to call multiple times. */
void srv_logger_close();

/** Perform rotation if current file exceeds maxBytes. Called automatically. */
void srv_logger_maybe_rotate();

} // namespace srv

// Resolve macro to file logging path. Always writes to stderr.
// File logging (when enabled) only happens for WARN/ERROR by default,
// but can be filtered by setting g_srvLogLevel.
// NOTE: the simple { ... } form is used (not do-while) to remain compatible
// with existing call sites in this codebase that omit the trailing semicolon,
// e.g.  void f() { SRV_LOGD("x") }.
#if 1
#define SRV_LOGD(...) { \
    ::srv::srv_logger_maybe_rotate(); \
    std::lock_guard<std::mutex> _srv_log_lk(::srv::g_srvLogMutex); \
    fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    if (::srv::g_srvLogFile && (int)::srv::SrvLogLevel::DEBUG >= (int)::srv::g_srvLogLevel) { \
        fprintf(::srv::g_srvLogFile, "D %s: Line %d:\t", __FILE__, __LINE__); \
        fprintf(::srv::g_srvLogFile, __VA_ARGS__); \
        fprintf(::srv::g_srvLogFile, "\n"); \
        fflush(::srv::g_srvLogFile); \
    } \
}

#define SRV_LOGI(...) { \
    ::srv::srv_logger_maybe_rotate(); \
    std::lock_guard<std::mutex> _srv_log_lk(::srv::g_srvLogMutex); \
    fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    if (::srv::g_srvLogFile && (int)::srv::SrvLogLevel::INFO >= (int)::srv::g_srvLogLevel) { \
        fprintf(::srv::g_srvLogFile, "I %s: Line %d:\t", __FILE__, __LINE__); \
        fprintf(::srv::g_srvLogFile, __VA_ARGS__); \
        fprintf(::srv::g_srvLogFile, "\n"); \
        fflush(::srv::g_srvLogFile); \
    } \
}

#define SRV_LOGW(...) { \
    ::srv::srv_logger_maybe_rotate(); \
    std::lock_guard<std::mutex> _srv_log_lk(::srv::g_srvLogMutex); \
    fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    if (::srv::g_srvLogFile && (int)::srv::SrvLogLevel::WARN >= (int)::srv::g_srvLogLevel) { \
        fprintf(::srv::g_srvLogFile, "W %s: Line %d:\t", __FILE__, __LINE__); \
        fprintf(::srv::g_srvLogFile, __VA_ARGS__); \
        fprintf(::srv::g_srvLogFile, "\n"); \
        fflush(::srv::g_srvLogFile); \
    } \
}

#define SRV_LOGE(...) { \
    ::srv::srv_logger_maybe_rotate(); \
    std::lock_guard<std::mutex> _srv_log_lk(::srv::g_srvLogMutex); \
    fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    if (::srv::g_srvLogFile && (int)::srv::SrvLogLevel::ERR >= (int)::srv::g_srvLogLevel) { \
        fprintf(::srv::g_srvLogFile, "E %s: Line %d:\t", __FILE__, __LINE__); \
        fprintf(::srv::g_srvLogFile, __VA_ARGS__); \
        fprintf(::srv::g_srvLogFile, "\n"); \
        fflush(::srv::g_srvLogFile); \
    } \
}
#else
#define SRV_LOGD(...)

#define SRV_LOGI(...)

#define SRV_LOGW(...)

#define SRV_LOGE(...)
#endif
