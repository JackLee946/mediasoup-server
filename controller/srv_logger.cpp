/************************************************************************
* @Copyright: 2023-2024
* @FileName: srv_logger.cpp
* @Description: File logger with level filter and size-based rotation.
* @Version: 1.0.0
* @Author: (auto)
* @CreateTime: 2026-07-17
*************************************************************************/

#include "srv_logger.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define SRV_DIR_SEPARATOR '\\'
#define SRV_PATH_MAX _MAX_PATH
#define srv_mkdir(p) _mkdir(p)
#define srv_stat _stat
#else
#include <unistd.h>
#define SRV_DIR_SEPARATOR '/'
#define SRV_PATH_MAX PATH_MAX
#define srv_mkdir(p) mkdir((p), 0755)
#define srv_stat stat
#endif

namespace srv {

FILE*        g_srvLogFile       = nullptr;
SrvLogLevel  g_srvLogLevel      = SrvLogLevel::INFO;
std::mutex   g_srvLogMutex;
std::size_t  g_srvLogMaxBytes   = 50 * 1024 * 1024;
int          g_srvLogBackupCount = 3;

// Stash the current log path so we can rotate by name.
static std::string s_logPath;

static void createParentDirs(const char* path)
{
    if (!path) {
        return;
    }
    std::string p(path);
    for (std::size_t i = 0; i < p.size(); ++i) {
        if (p[i] == '/' || p[i] == '\\') {
            if (i == 0) {
                continue;
            }
            std::string sub = p.substr(0, i);
            struct srv_stat st {};
            if (srv_stat(sub.c_str(), &st) != 0) {
                srv_mkdir(sub.c_str());
            }
        }
    }
}

static std::size_t fileSize(FILE* f)
{
    if (!f) {
        return 0;
    }
    long pos = ftell(f);
    if (fseek(f, 0, SEEK_END) != 0) {
        return 0;
    }
    long size = ftell(f);
    fseek(f, pos, SEEK_SET);
    return size > 0 ? static_cast<std::size_t>(size) : 0;
}

void srv_logger_maybe_rotate()
{
    if (!g_srvLogFile || g_srvLogMaxBytes == 0 || s_logPath.empty()) {
        return;
    }
    std::size_t size = fileSize(g_srvLogFile);
    if (size < g_srvLogMaxBytes) {
        return;
    }

    std::lock_guard<std::mutex> lk(g_srvLogMutex);
    // Re-check after acquiring the lock.
    size = fileSize(g_srvLogFile);
    if (size < g_srvLogMaxBytes) {
        return;
    }

    fflush(g_srvLogFile);
    fclose(g_srvLogFile);
    g_srvLogFile = nullptr;

    // Shift backups: .N -> deleted, .(N-1) -> .N, ..., .1 -> .2
    for (int i = g_srvLogBackupCount; i >= 1; --i) {
        std::string oldPath = s_logPath + "." + std::to_string(i);
        std::string newPath;
        if (i == g_srvLogBackupCount) {
            // Delete oldest
            std::remove(oldPath.c_str());
            continue;
        }
        newPath = s_logPath + "." + std::to_string(i + 1);
        std::rename(oldPath.c_str(), newPath.c_str());
    }
    // Current -> .1
    std::string firstBackup = s_logPath + ".1";
    std::rename(s_logPath.c_str(), firstBackup.c_str());

    g_srvLogFile = std::fopen(s_logPath.c_str(), "a");
    if (!g_srvLogFile) {
        // Failed to reopen: stderr remains the only sink.
        std::fprintf(stderr, "srv_logger: failed to reopen %s after rotation\n",
                     s_logPath.c_str());
    }
}

bool srv_logger_init(const char* path, SrvLogLevel level,
                     std::size_t maxBytes, int backupCount)
{
    std::lock_guard<std::mutex> lk(g_srvLogMutex);

    if (g_srvLogFile) {
        // Re-init: close existing handle first.
        std::fclose(g_srvLogFile);
        g_srvLogFile = nullptr;
        s_logPath.clear();
    }

    g_srvLogLevel      = level;
    g_srvLogMaxBytes   = maxBytes;
    g_srvLogBackupCount = backupCount > 0 ? backupCount : 1;

    if (!path || std::strlen(path) == 0) {
        // File logging explicitly disabled.
        return true;
    }

    createParentDirs(path);
    s_logPath = path;
    g_srvLogFile = std::fopen(path, "a");
    if (!g_srvLogFile) {
        std::fprintf(stderr, "srv_logger: failed to open %s for append\n", path);
        s_logPath.clear();
        return false;
    }
    // Make line-buffered to reduce data loss on crash.
    setvbuf(g_srvLogFile, nullptr, _IOLBF, 1024);
    return true;
}

void srv_logger_close()
{
    std::lock_guard<std::mutex> lk(g_srvLogMutex);
    if (g_srvLogFile) {
        std::fflush(g_srvLogFile);
        std::fclose(g_srvLogFile);
        g_srvLogFile = nullptr;
    }
    s_logPath.clear();
}

} // namespace srv
