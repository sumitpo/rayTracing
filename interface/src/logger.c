// logger.c
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

// ====== 配置：选择日志后端 ======
// 取消注释其中一行来切换实现
// #define USE_ZLOG
// #define USE_LOGC
#define USE_STDIO  // 默认使用 printf（无需依赖）

// ====== ZLOG 支持 ======
#ifdef USE_ZLOG
#include <zlog.h>
static zlog_category_t* zc = NULL;

int logger_init(const char* config_path) {
    if (zlog_init(config_path ? config_path : "zlog.conf") != 0) {
        fprintf(stderr, "Failed to init zlog\n");
        return -1;
    }
    zc = zlog_get_category("raytracer");
    if (!zc) {
        fprintf(stderr, "Failed to get zlog category\n");
        return -1;
    }
    return 0;
}

void logger_fini(void) {
    zlog_fini();
}

void logger_log(log_level_t level, const char* file, int line, const char* func, const char* fmt, ...) {
    if (!zc) return;

    va_list args;
    va_start(args, fmt);

    // 映射级别
    int zlog_level;
    switch (level) {
        case LOG_LEVEL_TRACE: zlog_level = ZLOG_DEBUG; break; // zlog 无 TRACE，用 DEBUG
        case LOG_LEVEL_DEBUG: zlog_level = ZLOG_DEBUG; break;
        case LOG_LEVEL_INFO:  zlog_level = ZLOG_INFO;  break;
        case LOG_LEVEL_WARN:  zlog_level = ZLOG_WARN;  break;
        case LOG_LEVEL_ERROR: zlog_level = ZLOG_ERROR; break;
        case LOG_LEVEL_FATAL: zlog_level = ZLOG_FATAL; break;
        default: zlog_level = ZLOG_INFO;
    }

    zlog(zc, zlog_level, file, line, func, fmt, args);
    va_end(args);
}

#elif defined(USE_LOGC)

// ====== log.c 支持 ======
// 假设你有一个 log.c / log.h（常见轻量级 C 日志库）
// 例如：https://github.com/rxi/log.c
#include "log.h"

int logger_init(const char* config_path) {
    // log.c 通常不需要初始化，或可通过 log_set_level() 设置
    log_set_level(LOG_INFO); // 默认
    return 0;
}

void logger_fini(void) {
    // log.c 通常无需清理
}

void logger_log(log_level_t level, const char* file, int line, const char* func, const char* fmt, ...) {
    int logc_level;
    switch (level) {
        case LOG_LEVEL_TRACE: logc_level = LOG_TRACE; break;
        case LOG_LEVEL_DEBUG: logc_level = LOG_DEBUG; break;
        case LOG_LEVEL_INFO:  logc_level = LOG_INFO;  break;
        case LOG_LEVEL_WARN:  logc_level = LOG_WARN;  break;
        case LOG_LEVEL_ERROR: logc_level = LOG_ERROR; break;
        case LOG_LEVEL_FATAL: logc_level = LOG_FATAL; break;
        default: logc_level = LOG_INFO;
    }

    va_list args;
    va_start(args, fmt);
    log_logv(logc_level, file, line, fmt, args);
    va_end(args);
}

#else

// ====== 默认：使用 stdio (printf) ======
static int global_log_level = LOG_LEVEL_INFO;

int logger_init(const char* config_path) {
    // 可选：从 config 解析日志级别
    global_log_level = LOG_LEVEL_INFO;
    return 0;
}

void logger_fini(void) {
    // nothing
}

const char* level_str(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_TRACE: return "TRACE";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

void logger_log(log_level_t level, const char* file, int line, const char* func, const char* fmt, ...) {
    if (level < global_log_level) return;

    // 获取时间
    time_t t = time(NULL);
    struct tm* tm_info = localtime(&t);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    // 打印前缀
    fprintf(stderr, "[%s] %s %s:%d %s(): ", time_str, level_str(level), file, line, func);

    // 打印消息
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);

    if (level == LOG_LEVEL_FATAL) {
        exit(EXIT_FAILURE);
    }
}

#endif
