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
#include <stdlib.h>  // getenv, strdup, free
#include <string.h>
#include <sys/stat.h>  // stat, S_ISDIR -- DIRECTORY validation

#include "log.h"
#include "project_config.h"

// True once the user passed an explicit DIRECTORY positional; decides
// CLI-vs-environment precedence in command_line_dirs().
static bool G_dir_explicit = false;

// Owns the strdup'd SIFT_DIRS copy that command_line_dirs()' tokens
// point into; process lifetime, freed by the OS at exit.
static char *G_dir_list = NULL;

// True if path exists and is a directory (symlinks to dirs pass).
static bool is_dir(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

// Shown by argp in --help/version/bug output.
const char *argp_program_version     = MAIN_BINARY " " PROJECT_VERSION;
const char *argp_program_bug_address = PROJECT_HOMEPAGE_URL "/issues"
                                                            "\n" AUTH_MESSAGE;
static char args_doc[]               = "[DIRECTORY]";  // non-option args line in --help
static char doc[]                    = MAIN_BINARY " - " PROJECT_DESCRIPTION;

// Terminated by the all-zero entry argp requires. Non-zero group ids
// pair each option with its header row; same id = printed under that
// heading in --help.
static struct argp_option options[] = {
	{ 0, 0, 0, 0, "Logging:", 1 },
	{ "log-level",      'L',  "LEVEL",  0,  "Set log level: [off|fatal|error|warn|info|debug|trace] (default: info)", 1 },
	{ "log-file",       'F',  "FILE",   0,  "Set logging file",                                                       1 },

	{ 0, 0, 0, 0, "Scanning:", 2 },
	{ "mime-type",      'M',  0,        0,  "Detect file type by content (magic bytes), not just name",               2 },
	{ "dry-run",        'n',  0,        0,  "Show what would move without moving anything",                           2 },

	{ 0, 0, 0, 0, "Information:", 3 },
	{ "rules",          'R',  0,        0,  "Print the current rule list and exit.",                                  3 },

	{ 0 }
};

// Defaults applied before parsing; parse_opt overwrites per flag seen.
Command_line_options G_args = {
	.print_rules = false,
	.detect_mime = false,
	.dry_run     = false,
	.dir         = ".",
	.log_file    = NULL,
	.log_level   = LOG_LEVEL_INFO,
};

/*
 * Called by argp for every short/long option encountered.
 * Returning ARGP_ERR_UNKNOWN defers to argp's own error handling;
 * ARGP_KEY_ARG delivers one non-option word (the positional
 * DIRECTORY), ARGP_KEY_END marks the end of parsing.
 */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	switch (key) {
		case 'R':  G_args.print_rules = true; break;
		case 'M':  G_args.detect_mime = true; break;
		case 'n':  G_args.dry_run     = true; break;
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
		case ARGP_KEY_ARG:
			// Only one positional is accepted; extras are a usage error.
			if (state->arg_num > 0)
				argp_error(state, "Too many arguments: only one DIRECTORY is supported");
			G_args.dir      = arg;
			G_dir_explicit  = true;
			break;
		case ARGP_KEY_END:
			// Validate at the boundary so the scan loop can trust dir.
			if (G_dir_explicit && !is_dir(G_args.dir))
				argp_error(state, "'%s' is not an existing directory", G_args.dir);
			break;
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

/*
 * Resolve which directories to process.
 *
 * Precedence: explicit DIRECTORY argument beats the SIFT_DIRS
 * environment variable (";"-separated list); with neither, fall back
 * to ".". Invalid SIFT_DIRS entries are skipped with a warning.
 * SIFT_DIRS="" behaves as unset.
 *
 * Returns a malloc'd NULL-terminated array of paths (never NULL when
 * count > 0 is possible -- returns NULL only if nothing usable was
 * found; the reason is already logged). The strings live for the
 * process lifetime: free() the array itself, never its elements.
 */
const char **command_line_dirs(void)
{
	const char *sift_dirs = getenv("SIFT_DIRS");

	if (!G_dir_explicit && sift_dirs != NULL && sift_dirs[0] != '\0') {
		// strtok() writes separators, and getenv()'s buffer must not be
		// modified -- work on our own copy.
		G_dir_list = strdup(sift_dirs);

		// N tokens need at most N separators + 1; over-allocating a few
		// pointers beats growing on the fly.
		size_t max        = 2;
		const char **list = NULL;
		for (const char *p = sift_dirs; *p != '\0'; p++)
			if (*p == ';') max++;

		list = malloc(max * sizeof(*list));
		size_t n = 0;

		for (char *token = strtok(G_dir_list, ";"); token != NULL; token = strtok(NULL, ";")) {
			if (!is_dir(token)) {
				LOG_WARN("SIFT_DIRS: skipping '%s' (not an existing directory)", token);
				continue;
			}
			list[n++] = token;
		}

		if (n == 0) {
			LOG_ERROR("SIFT_DIRS set but contains no usable directory");
			free(list);
			return NULL;
		}

		list[n] = NULL;
		return list;
	}

	// Explicit argument or plain default: a single-entry list.
	const char **list = malloc(2 * sizeof(*list));
	list[0]           = G_args.dir;
	list[1]           = NULL;
	return list;
}
