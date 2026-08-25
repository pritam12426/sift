#ifndef _COMMAND_LINE_H_
#define _COMMAND_LINE_H_


#include <stdbool.h>

#include "log.h"

typedef struct {
	bool        print_request;
	const char *dir;
	const char *browser;
	const char *host;
	const char *log_file;
	Log_level_t log_level;
} Command_line_options;

extern Command_line_options G_args;

bool command_line_parse(int argc, char **argv);


#endif  // _COMMAND_LINE_H_
