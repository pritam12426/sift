/*
 * Copyright (c) 2026 Pritam
 *
 * SPDX-License-Identifier: MIT
 */


/*
 * log.h — Thread-safe logger interface
 *
 * Logging macros (severity, high → low):
 *   LOG_FATAL(...)  — unrecoverable errors
 *   LOG_ERROR(...)  — recoverable errors
 *   LOG_WARN(...)   — warnings
 *   LOG_INFO(...)   — informational messages
 *   LOG_DEBUG(...)  — debug details
 *   LOG_TRACE(...)  — fine-grained tracing
 *
 * Build-time flags (set via -D in Makefile):
 *   LOG_SHOW_SOURCE_LOCATION   — append [file:line:func]
 *
 * Usage:
 *   LOG_INFO("server started on port %d", 8080);
 */

#ifndef _LOG__H_
#define _LOG__H_


#include <stdbool.h> // bool
#include <stdio.h>   // FILE

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


// Log severity levels (lower number = higher priority)
typedef enum {
	LOG_LEVEL_OFF   = 0,
	LOG_LEVEL_FATAL = 1,
	LOG_LEVEL_ERROR = 2,
	LOG_LEVEL_WARN  = 3,
	LOG_LEVEL_INFO  = 4,
	LOG_LEVEL_DEBUG = 5,
	LOG_LEVEL_TRACE = 6,
} Log_level_t;


// Logger configuration
void log_set_level(Log_level_t level);
Log_level_t log_get_level(void);

// Returns true if the logger is currently emitting ANSI color codes.
// Color is enabled automatically when the output stream is a TTY,
// and disabled when writing to a file.
bool log_use_color(void);

// Internal implementation — do not call directly.
void log_record(Log_level_t level,
                const char *file __attribute__((unused)),
                int         line __attribute__((unused)),
                const char *func __attribute__((unused)),
                int         new_line,
                const char *fmt,
                ...);



// Public logging macros
// Check if messages at the given level would be emitted right now.
#define LOG_LEVEL_IS_ENABLED(level) (log_get_level() >= (level))


#ifdef LOG_SHOW_SOURCE_LOCATION

// Log with custom newline behaviour (0 = no newline, 1 = with newline)
// Used internally; prefer LOG_FATAL / LOG_ERROR / LOG_WARN / etc.
#define LOG_CUSTOM(LOG_LEVEL, NEW_LINE, ...)                                   \
	log_record(LOG_LEVEL, __FILE__, __LINE__, __func__, NEW_LINE, __VA_ARGS__)

// Log an error and append strerror(errno) (via perror)
#define LOG_PERROR(...)                                                            \
	do {                                                                           \
		log_record(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, 0, __VA_ARGS__); \
		perror(" ");                                                               \
	} while (0)

#define LOG_FATAL(...) \
	log_record(LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, 1, __VA_ARGS__)

#define LOG_ERROR(...) \
	log_record(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, 1, __VA_ARGS__)

#define LOG_WARN(...) \
	log_record(LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, 1, __VA_ARGS__)

#define LOG_INFO(...) \
	log_record(LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, 1, __VA_ARGS__)

#define LOG_DEBUG(...) \
	log_record(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, 1, __VA_ARGS__)

#define LOG_TRACE(...) \
	log_record(LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, 1, __VA_ARGS__)

#else

// Log with custom newline behaviour (0 = no newline, 1 = with newline)
// Used internally; prefer LOG_FATAL / LOG_ERROR / LOG_WARN / etc.
#define LOG_CUSTOM(LOG_LEVEL, NEW_LINE, ...)                                   \
	log_record(LOG_LEVEL, 0, 0, 0, NEW_LINE, __VA_ARGS__)

// Log an error and append strerror(errno) (via perror)
#define LOG_PERROR(...)                                                            \
	do {                                                                           \
		log_record(LOG_LEVEL_ERROR, 0, 0, 0, 0, __VA_ARGS__); \
		perror(" ");                                                               \
	} while (0)

#define LOG_FATAL(...) \
	log_record(LOG_LEVEL_FATAL, 0, 0, 0, 1, __VA_ARGS__)

#define LOG_ERROR(...) \
	log_record(LOG_LEVEL_ERROR, 0, 0, 0, 1, __VA_ARGS__)

#define LOG_WARN(...) \
	log_record(LOG_LEVEL_WARN, 0, 0, 0, 1, __VA_ARGS__)

#define LOG_INFO(...) \
	log_record(LOG_LEVEL_INFO, 0, 0, 0, 1, __VA_ARGS__)

#define LOG_DEBUG(...) \
	log_record(LOG_LEVEL_DEBUG, 0, 0, 0, 1, __VA_ARGS__)

#define LOG_TRACE(...) \
	log_record(LOG_LEVEL_TRACE, 0, 0, 0, 1, __VA_ARGS__)

#endif  // LOG_SHOW_SOURCE_LOCATION


#ifdef __cplusplus
}
#endif  // __cplusplus


#endif  // _LOG__H_
