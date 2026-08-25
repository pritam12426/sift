/*
 * Copyright (c) 2026 Pritam
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * main.c -- sift entry point
 *
 * Flow: parse args -> handle -R short-circuit -> scan/move files.
 */

#include <stdio.h>
#include <stdlib.h>  // free

#include "command_line.h"
#include "log.h"
#include "rules.h"
#include "scan.h"

int main(int argc, char *argv[])
{
	if (!command_line_parse(argc, argv))
		return 1;

	// Apply the log level from -L/--log-level (parsed but not yet
	// connected to the logger).
	log_set_level(G_args.log_level);

	// -R wins over everything else: show rules and exit cleanly.
	if (G_args.print_rules) {
		rules_print();
		return 0;
	}

	// Echo argv when debugging (LOG_CUSTOM without newline; we add
	// our own separators between arguments).
	if (LOG_LEVEL_IS_ENABLED(LOG_LEVEL_DEBUG)) {
		LOG_CUSTOM(LOG_LEVEL_DEBUG, false, "Command-line args: [");
		for (int i = 0; i < argc; i++) {
			fprintf(stderr, "\"%s\"", argv[i]);
			if (i != argc - 1)
				fputs(", ", stderr);
		}
		fputs("]\n", stderr);
	}

	// Resolve the target list: explicit DIRECTORY beats SIFT_DIRS,
	// which beats the "." default. NULL means nothing usable -> logged
	// already, exit non-zero.
	const char **dirs = command_line_dirs();
	if (dirs == NULL) {
		return 1;
	}

	for (size_t i = 0; dirs[i] != NULL; i++) {
		scan_dir(dirs[i]);
	}

	free(dirs);  // element strings are process-lifetime, array is ours

	return 0;
}
