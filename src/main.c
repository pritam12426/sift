/*
 * Copyright (c) 2026 Pritam
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * main.c -- sift entry point
 *
 * Flow: parse args -> handle -R short-circuit -> scan/move files.
 * The scan loop is not implemented yet; -R is the first complete path.
 */

#include <stdio.h>

#include "command_line.h"
#include "log.h"
#include "rules.h"

int main(int argc, char *argv[])
{
	if (!command_line_parse(argc, argv))
		return 1;

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

	// TODO(scan): iterate DIRECTORY entries at depth 1, find_rule()
	// each one (reading MAGIC_HEADER_SIZE bytes when detect_mime is
	// set), and move matches into their dest folder.
	LOG_INFO("Hello this pritam");

	return 0;
}
