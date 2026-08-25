#include "rules.h"

#include <stdio.h>
#include <string.h>

const Rule G_rules[] = {
	// --- Starts with ---
	{ "WhatsApp", "whatsapp", MATCH_STARTS },
	{ "IMG_",     "img",      MATCH_STARTS },
	{ "DSC_",     "photos",   MATCH_STARTS },
	{ "VID_",     "videos",   MATCH_STARTS },

	// --- Contains (using strstr) ---
	{ "backup",  "backups", MATCH_CONTAINS },
	{ "temp",    "tmp",     MATCH_CONTAINS },
	{ "draft",   "drafts",  MATCH_CONTAINS },

	// --- Ends with (extensions) ---
	{ ".png",    "img",     MATCH_ENDS },
	{ ".jpg",    "img",     MATCH_ENDS },
	{ ".jpeg",   "img",     MATCH_ENDS },
	{ ".gif",    "img",     MATCH_ENDS },
	{ ".bmp",    "img",     MATCH_ENDS },
	{ ".pdf",    "doc",     MATCH_ENDS },
	{ ".doc",    "doc",     MATCH_ENDS },
	{ ".docx",   "doc",     MATCH_ENDS },
	{ ".ppt",    "doc",     MATCH_ENDS },
	{ ".pptx",   "doc",     MATCH_ENDS },
	{ ".xls",    "doc",     MATCH_ENDS },
	{ ".xlsx",   "doc",     MATCH_ENDS },
	{ ".mp3",    "audio",   MATCH_ENDS },
	{ ".wav",    "audio",   MATCH_ENDS },
	{ ".mp4",    "video",   MATCH_ENDS },
	{ ".avi",    "video",   MATCH_ENDS }
};


int matches_rule(const char *filename, const Rule *rule)
{
	size_t f_len = strlen(filename);
	size_t p_len = strlen(rule->pattern);

	switch (rule->type) {
		case MATCH_STARTS: {
			return (f_len >= p_len && strncmp(filename, rule->pattern, p_len) == 0);
		}
		case MATCH_CONTAINS: {
			return (strstr(filename, rule->pattern) != NULL);
		}
		case MATCH_ENDS: {
			return (f_len >= p_len && strcmp(filename + f_len - p_len, rule->pattern) == 0);
		}
		default:
			return 0;
	}
}

void rules_print(void)
{
	static const char *type_names[] = { "starts", "contains", "ends" };

	printf("Rules (first match wins):\n");
	for (size_t i = 0; i < RULES_COUNT; i++) {
		const Rule *r = &G_rules[i];
		printf("  %-8s %-10s -> %s/\n", type_names[r->type], r->pattern, r->dest);
	}
}
