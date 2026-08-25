#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#define COLOR_RESET        "\x1b[0m"
#define COLOR_BOLD_RED     "\x1b[1;31m"
#define COLOR_BOLD_GREEN   "\x1b[1;32m"
#define COLOR_BOLD_YELLOW  "\x1b[1;33m"
#define COLOR_BOLD_BLUE    "\x1b[1;34m"
#define COLOR_BOLD_MAGENTA "\x1b[1;35m"
#define COLOR_BOLD_CYAN    "\x1b[1;36m"
#define COLOR_DIM          "\x1b[2m"


// -- State ---------------------------------------------------------
static Log_level_t G_log_level = LOG_LEVEL_INFO;
int                G_use_color = 0;

void log_set_level(Log_level_t level) { G_log_level = level; }
Log_level_t log_get_level(void)       { return G_log_level;  }


// -- Level metadata ------------------------------------------------
typedef struct {
	const char *label;
	const char *color;
} Level_meta_t;

static const Level_meta_t G_level_meta[] = {
	[LOG_LEVEL_FATAL] = { .label = "FATAL", .color = COLOR_BOLD_RED    },
	[LOG_LEVEL_ERROR] = { .label = "ERROR", .color = COLOR_BOLD_RED    },
	[LOG_LEVEL_WARN]  = { .label = "WARN ", .color = COLOR_BOLD_YELLOW },
	[LOG_LEVEL_INFO]  = { .label = "INFO ", .color = COLOR_BOLD_GREEN  },
	[LOG_LEVEL_DEBUG] = { .label = "DEBUG", .color = COLOR_BOLD_CYAN   },
	[LOG_LEVEL_TRACE] = { .label = "TRACE", .color = COLOR_BOLD_CYAN   },
};


// -- log_record ----------------------------------------------------
void log_record(Log_level_t level,
                const char *file __attribute__((unused)),
                int         line __attribute__((unused)),
                const char *func __attribute__((unused)),
                int         new_line,
                const char *fmt,
                ...)
{
	if (level > G_log_level) return;

	if (G_use_color == 0)
		G_use_color = isatty(fileno(stderr)) ? 1 : -1;

	const Level_meta_t *m = &G_level_meta[level];

	if (G_use_color > 0) {
		fprintf(stderr, "[%s%s" COLOR_RESET "] ", m->color, m->label);
	} else {
		fprintf(stderr, "[%s] ", m->label);
	}

#ifdef LOG_SHOW_SOURCE_LOCATION
	if (G_use_color > 0) {
		fprintf(stderr, COLOR_DIM "[%s:%d:%s]" COLOR_RESET " ", file, line, func);
	} else {
		fprintf(stderr, "[%s:%d:%s] ", file, line, func);
	}
#endif  // LOG_SHOW_SOURCE_LOCATION

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (new_line) fputc('\n', stderr);
}
