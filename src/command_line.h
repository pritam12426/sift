/*
 * Copyright (c) 2026 Pritam
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * command_line.h -- CLI option state
 *
 * G_args is the single global holding everything the command line set;
 * read it instead of passing options around.
 */

#ifndef _COMMAND_LINE_H_
#define _COMMAND_LINE_H_


#include <stdbool.h>

#include "log.h"

typedef struct {
	bool        print_rules;  // -R: print rule table and exit
	bool        detect_mime;  // -M: sniff file content, not just name
	const char *log_file;     // -F: log destination file, NULL = stderr
	Log_level_t log_level;    // -L: minimum severity that gets printed
} Command_line_options;

// Defined in command_line.c; filled in by command_line_parse().
extern Command_line_options G_args;

// Parse argv into G_args. false only on internal failure -- argp exits
// the process itself for -h/--usage bad-input cases.
bool command_line_parse(int argc, char **argv);


#endif  // _COMMAND_LINE_H_
