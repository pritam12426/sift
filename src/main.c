#include <stdio.h>

#include "command_line.h"
#include "log.h"
#include "rules.h"

int main(int argc, char *argv[])
{
	if (!command_line_parse(argc, argv)) return 1;

	if (G_args.print_rules) {
		rules_print();
		return 0;
	}

	if (LOG_LEVEL_IS_ENABLED(LOG_LEVEL_DEBUG)) {
		LOG_CUSTOM(LOG_LEVEL_DEBUG, false, "Command-line args: [");
		for (int i = 0; i < argc; i++) {
			fprintf(stderr, "\"%s\"", argv[i]);
			if (i != argc - 1) fputs(", ", stderr);
		}
		fputs("]\n", stderr);
	}

	LOG_INFO("Hello this pritam");

	return 0;
}
