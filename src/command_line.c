/*
 * Copyright (c) 2026 Pritam
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * command_line.c -- argp-based CLI parsing
 *
 * Parses argv into the global G_args. argp handles -h/--help,
 * --usage and --version for us; this file only declares the options
 * table and maps each key to a field assignment.
 */

#include "command_line.h"

#include <argp.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "project_config.h"

// Shown by argp in --help/version/bug output.
const char *argp_program_version     = MAIN_BINARY " " PROJECT_VERSION;
const char *argp_program_bug_address = PROJECT_HOMEPAGE_URL "/issues"
                                                            "\n" AUTH_MESSAGE;
static char args_doc[]               = "[DIRECTORY]";  // non-option args line in --help
static char doc[]                    = MAIN_BINARY " - " PROJECT_DESCRIPTION;

// Terminated by the all-zero entry argp requires; group field left 0
// so options print ungrouped.
static struct argp_option options[] = {
	{ "log-level",      'L',  "LEVEL",  0,  "Set log level: [off|fatal|error|warn|info|debug|trace] (default: info)",  },
	{ "log-file",       'F',  "FILE",   0,  "Set logging file",                                                        },
	{ "rules",          'R',  0,        0,  "Print the current rule list and exit.",                                   },
	{ "mime-type",      'M',  0,        0,  "Detect file type by content (magic bytes), not just name",                },

	{ 0 }
};

// Defaults applied before parsing; parse_opt overwrites per flag seen.
Command_line_options G_args = {
	.print_rules = false,
	.detect_mime = false,
	.log_file    = NULL,
	.log_level   = LOG_LEVEL_INFO,
};

/*
 * Called by argp for every short/long option encountered.
 * Returning ARGP_ERR_UNKNOWN defers to argp's own error handling;
 * ARGP_KEY_END marks the end of parsing (nothing to do yet).
 */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	switch (key) {
		case 'R':  G_args.print_rules = true; break;
		case 'M':  G_args.detect_mime = true; break;
		case 'F':  G_args.log_file    = arg;  break;
		case 'L': {
			if      (strcmp(arg, "off")   == 0)  G_args.log_level = LOG_LEVEL_OFF;
			else if (strcmp(arg, "fatal") == 0)  G_args.log_level = LOG_LEVEL_FATAL;
			else if (strcmp(arg, "error") == 0)  G_args.log_level = LOG_LEVEL_ERROR;
			else if (strcmp(arg, "warn")  == 0)  G_args.log_level = LOG_LEVEL_WARN;
			else if (strcmp(arg, "info")  == 0)  G_args.log_level = LOG_LEVEL_INFO;
			else if (strcmp(arg, "debug") == 0)  G_args.log_level = LOG_LEVEL_DEBUG;
			else if (strcmp(arg, "trace") == 0)  G_args.log_level = LOG_LEVEL_TRACE;
			else argp_error(state, "Invalid log level: '%s'. Use: off, fatal, error, warn, info, debug, trace.", arg);
			break;
		}
		case ARGP_KEY_END: break;
		default: return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = { .options = options, .parser = parse_opt, .args_doc = args_doc, .doc = doc };

// Returns false only on internal parse failure; -h/-V etc. exit inside
// argp and never reach here.
bool command_line_parse(int argc, char **argv)
{
	error_t err = argp_parse(&argp, argc, argv, 0, 0, 0);
	if (err != 0) {
		fprintf(stderr, MAIN_BINARY " : failed to parse command-line arguments\n");
		return false;
	}
	return true;
}
