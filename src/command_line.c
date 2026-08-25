#include "command_line.h"

#include <argp.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "project_config.h"

const char *argp_program_version     = MAIN_BINARY " " PROJECT_VERSION;
const char *argp_program_bug_address = PROJECT_HOMEPAGE_URL "/issues" "\n" AUTH_MESSAGE;
static char args_doc[]               = "[DIRECTORY]";
static char doc[]                    = MAIN_BINARY " - " PROJECT_DESCRIPTION;

static struct argp_option options[] = {
	{ "log-level",      'L',  "LEVEL",  0,  "Set log level: [off|fatal|error|warn|info|debug|trace] (default: info)",  },
	{ "log-file",       'F',  "FILE",   0,  "Set logging file",                                                        },
	{ "rules",          'R',  0,        0,  "Print the current rule list and exit.",                                   },
	{ "mime-type",      'M',  0,        0,  "Detect file type by content (magic bytes), not just name",                },

	{ 0 }
};

Command_line_options G_args = {
	.print_rules = false,
	.detect_mime = false,
	.log_file    = NULL,
	.log_level   = LOG_LEVEL_INFO,
};

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

bool command_line_parse(int argc, char **argv)
{
	error_t err = argp_parse(&argp, argc, argv, 0, 0, 0);
	if (err != 0) {
		fprintf(stderr, MAIN_BINARY " : failed to parse command-line arguments\n");
		return false;
	}
	return true;
}
