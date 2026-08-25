/*
 * Copyright (c) 2026 Pritam
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * rules.c -- Rule table, matching, and content sniffing
 */

#include "rules.h"

#include <stdio.h>
#include <string.h>

#include "log.h"

// Private to this file on purpose: sizeof(G_rules) is only valid where
// the array is defined; other translation units see an incomplete type.
#define RULES_COUNT (sizeof(G_rules) / sizeof(Rule))

/*
 * The rule table. Order is priority: the first rule that matches a
 * file (by name, or by content when -M is active) decides its
 * destination.
 *
 * Signature notes:
 *  - Signatures are matched with memmem() over the header, NOT prefix
 *    memcmp(), because some sit at an offset ("WAVE"/"AVI " at byte 8,
 *    "ftyp" at byte 4).
 *  - Shared signatures are stored once: .doc carries the OLE2 header
 *    (also recognizes legacy .xls/.ppt), and .docx carries the ZIP
 *    local-file-header (also recognizes .xlsx/.pptx). Both groups map
 *    to doc/, so first-match-wins keeps routing correct.
 *  - Write signatures as one explicit \xNN per byte. C hex escapes
 *    swallow following hex digits ("\x00A" is a single byte 0x0A).
 */
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

// Name-pattern matching only; content is matches_magic()'s job.
int matches_rule(const char *filename, const Rule *rule)
{
	size_t f_len = strlen(filename);
	size_t p_len = strlen(rule->pattern);

	switch (rule->type) {
		case MATCH_STARTS: {  // prefix compare, needs room for the whole pattern
			return (f_len >= p_len && strncmp(filename, rule->pattern, p_len) == 0);
		}
		case MATCH_CONTAINS: {  // substring anywhere; strstr handles lengths
			return (strstr(filename, rule->pattern) != NULL);
		}
		case MATCH_ENDS: {  // compare the filename's tail against the pattern
			return (f_len >= p_len && strcmp(filename + f_len - p_len, rule->pattern) == 0);
		}
		default:
			return 0;
	}
}

int matches_magic(const unsigned char *header, size_t header_len, const Rule *rule)
{
	// No signature, empty signature, or truncated read -> never match.
	if (!rule->magic_bytes || rule->magic_len == 0 || header_len < rule->magic_len)
		return 0;
	return memmem(header, header_len, rule->magic_bytes, rule->magic_len) != NULL;
}

const Rule *
find_rule(const char *filename, const unsigned char *header, size_t header_len, bool use_magic)
{
	LOG_TRACE("find_rule: filename='%s', header_len=%zu, use_magic=%d", filename, header_len,
	          (int)use_magic);

	// Single pass keeps table order as the only priority mechanism:
	// each rule is tried by name, then by content if enabled.
	for (size_t i = 0; i < RULES_COUNT; i++) {
		const Rule *r = &G_rules[i];
		if (matches_rule(filename, r)) {
			LOG_DEBUG("Rule matched '%s': name pattern '%s' -> %s/", filename, r->pattern, r->dest);
			return r;
		}
		if (use_magic && matches_magic(header, header_len, r)) {
			LOG_DEBUG("Rule matched '%s': magic bytes of '%s' -> %s/", filename, r->pattern, r->dest);
			return r;
		}
	}

	LOG_DEBUG("No matching rule for '%s'", filename);
	return NULL;
}

void rules_print(void)
{
	static const char *type_names[] = { "starts", "contains", "ends" };

	printf("Rules (first match wins):\n");
	for (size_t i = 0; i < RULES_COUNT; i++) {
		const Rule *r = &G_rules[i];
		printf("  %-10s %-10s -> %s/\n", type_names[r->type], r->pattern, r->dest);
	}
}
