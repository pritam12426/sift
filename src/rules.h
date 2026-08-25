/*
 * Copyright (c) 2026 Pritam
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * rules.h -- Pattern-match rules and magic-byte detection
 *
 * A Rule pairs a filename pattern (starts-with / contains / ends-with)
 * with a destination folder and an optional magic-byte signature used to
 * recognize a file by its content (--mime-type / -M).
 *
 * All rules live in G_rules[] and are evaluated first-match-wins:
 * table order is priority order.
 */

#ifndef _RULES_H_
#define _RULES_H_

#include <stdbool.h>  // bool
#include <stddef.h>   // size_t

// Max bytes read from a file's start for content sniffing.
// Signatures must occur within this window to be found.
#define MAGIC_HEADER_SIZE 16

typedef enum {
	MATCH_STARTS,    // filename begins with pattern
	MATCH_CONTAINS,  // filename contains pattern anywhere (strstr)
	MATCH_ENDS       // filename ends with pattern (e.g. extensions)
} MatchType;

typedef struct {
	const char *pattern;      // name pattern to match against
	const char *dest;         // destination folder if the rule wins
	MatchType   type;         // how pattern is compared to the filename
	const char *magic_bytes;  // raw signature bytes, or NULL for none
	size_t      magic_len;    // length of the signature in bytes
} Rule;

extern const Rule G_rules[];

// True (non-zero) if filename matches the rule's name pattern.
int matches_rule(const char *filename, const Rule *rule);

// Search a file's header for the rule's signature. Returns 0 when the
// rule has no signature or fewer bytes than that were read, so
// name-only rules never match on content.
int matches_magic(const unsigned char *header, size_t header_len, const Rule *rule);

// First rule matching this entry (name first, then --magic if
// use_magic is set), or NULL for no match. header may be NULL with
// header_len 0 when no bytes were read.
const Rule *
find_rule(const char *filename, const unsigned char *header, size_t header_len, bool use_magic);

// Print G_rules[] as an aligned table to stdout (-R/--rules).
void rules_print(void);

#endif  // _RULES_H_
