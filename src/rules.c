#include "rules.h"

#include <stdio.h>
#include <string.h>

const Rule G_rules[] = {
	// --- Starts with ---
	{ "WhatsApp", "whatsapp", MATCH_STARTS, NULL, 0 },
	{ "IMG_",     "img",      MATCH_STARTS, NULL, 0 },
	{ "DSC_",     "photos",   MATCH_STARTS, NULL, 0 },
	{ "VID_",     "videos",   MATCH_STARTS, NULL, 0 },

	// --- Contains (using strstr) ---
	{ "backup",  "backups", MATCH_CONTAINS, NULL, 0 },
	{ "temp",    "tmp",     MATCH_CONTAINS, NULL, 0 },
	{ "draft",   "drafts",  MATCH_CONTAINS, NULL, 0 },

	// --- Ends with (extensions) ---
	{ ".png",  "img",   MATCH_ENDS, "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a",        8 },
	{ ".jpg",  "img",   MATCH_ENDS, "\xff\xd8\xff",                            3 },
	{ ".jpeg", "img",   MATCH_ENDS, NULL,                                      0 },
	{ ".gif",  "img",   MATCH_ENDS, "GIF8",                                    4 },
	{ ".bmp",  "img",   MATCH_ENDS, "BM",                                      2 },
	{ ".pdf",  "doc",   MATCH_ENDS, "%PDF",                                    4 },
	{ ".doc",  "doc",   MATCH_ENDS, "\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1",        8 },
	{ ".docx", "doc",   MATCH_ENDS, "PK\x03\x04",                              4 },
	{ ".ppt",  "doc",   MATCH_ENDS, NULL,                                      0 },
	{ ".pptx", "doc",   MATCH_ENDS, NULL,                                      0 },
	{ ".xls",  "doc",   MATCH_ENDS, NULL,                                      0 },
	{ ".xlsx", "doc",   MATCH_ENDS, NULL,                                      0 },
	{ ".mp3",  "audio", MATCH_ENDS, "ID3",                                     3 },
	{ ".wav",  "audio", MATCH_ENDS, "WAVE",                                    4 },
	{ ".mp4",  "video", MATCH_ENDS, "ftyp",                                    4 },
	{ ".avi",  "video", MATCH_ENDS, "AVI ",                                    4 }
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
