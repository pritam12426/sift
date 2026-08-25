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
	bool        dry_run;      // -n: preview moves without touching files
	const char *dir;          // positional DIRECTORY to process, "." if omitted
	const char *log_file;     // -F: log destination file, NULL = stderr
	Log_level_t log_level;    // -L: minimum severity that gets printed
} Command_line_options;

// Defined in command_line.c; filled in by command_line_parse().
extern Command_line_options G_args;

// Parse argv into G_args. false only on internal failure -- argp exits
// the process itself for -h/--usage bad-input cases.
bool command_line_parse(int argc, char **argv);

// Resolve which directories to process: explicit DIRECTORY beats the
// SIFT_DIRS env var (";"-separated); neither -> ["."]. Returns a
// malloc'd NULL-terminated array, or NULL if nothing usable was found
// (reason already logged). Free the array itself; its strings are
// process-lifetime and must not be freed.
const char **command_line_dirs(void);


#endif  // _COMMAND_LINE_H_
