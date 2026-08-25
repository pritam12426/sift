#ifndef _RULES_H_
#define _RULES_H_


#define RULES_COUNT (sizeof(G_rules) / sizeof(Rule))

typedef enum {
	MATCH_STARTS,    // filename begins with pattern
	MATCH_CONTAINS,  // filename contains pattern (strstr)
	MATCH_ENDS       // filename ends with pattern
} MatchType;

typedef struct {
	const char *pattern;
	const char *dest;
	MatchType   type;
} Rule;


extern const Rule G_rules[];

int  matches_rule(const char *filename, const Rule *rule);
void rules_print(void);

#endif  // _RULES_H_
