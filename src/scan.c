/*
 * Copyright (c) 2026 Pritam
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * scan.c -- Depth-1 scan: match files, create dest folders, move
 */

#include "scan.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>  // rename

#include "command_line.h"
#include "log.h"
#include "rules.h"

#ifndef PATH_MAX
#define PATH_MAX 4096  // not guaranteed by POSIX; sane fallback
#endif

// Read up to MAGIC_HEADER_SIZE bytes from the file's start.
// Returns how many bytes were read (0 if unreadable).
static size_t read_magic(const char *path, unsigned char *buf)
{
	FILE *fp = fopen(path, "rb");
	if (fp == NULL) return 0;

	size_t n = fread(buf, 1, MAGIC_HEADER_SIZE, fp);
	fclose(fp);
	return n;
}

// Make sure '<dir>/<dest>' exists, creating it on first need so empty
// categories never materialize. Dry-run never touches the filesystem;
// a missing folder is treated as fine because nothing will be moved.
static bool ensure_dest(const char *dir, const char *dest)
{
	char path[PATH_MAX];
	snprintf(path, sizeof(path), "%s/%s", dir, dest);

	struct stat st;
	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) return true;

	if (G_args.dry_run) return true;

	return mkdir(path, 0755) == 0 || errno == EEXIST;
}

int scan_dir(const char *dir)
{
	DIR *dp = opendir(dir);
	if (dp == NULL) {
		LOG_PERROR("opendir '%s'", dir);
		return -1;
	}

	LOG_INFO("Scanning '%s'", dir);

	struct dirent   *ep;
	struct stat      st;
	unsigned char    header[MAGIC_HEADER_SIZE];
	char             src[PATH_MAX];
	char             dst[PATH_MAX];

	while ((ep = readdir(dp)) != NULL) {
		const char *name = ep->d_name;
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

		int w = snprintf(src, sizeof(src), "%s/%s", dir, name);
		if (w < 0 || w >= (int)sizeof(src)) {
			LOG_WARN("Path too long, skipping '%s/%s'", dir, name);
			continue;
		}

		// Depth-1 means regular files only. d_type is a fast path;
		// DT_UNKNOWN filesystems fall back to stat().
		bool is_regular = ep->d_type == DT_REG;
		if (ep->d_type == DT_UNKNOWN) {
			if (stat(src, &st) != 0) {
				LOG_WARN("Cannot stat '%s', skipping", src);
				continue;
			}
			is_regular = S_ISREG(st.st_mode);
		}
		if (!is_regular) continue;  // dirs/symlinks/specials stay untouched

		// Content sniffing only when asked (-M); otherwise no open().
		const unsigned char *header_p = NULL;
		size_t header_len             = 0;
		if (G_args.detect_mime) {
			header_len = read_magic(src, header);
			header_p   = header;
		}

		const Rule *rule = find_rule(name, header_p, header_len, G_args.detect_mime);
		if (rule == NULL) {
			LOG_TRACE("No rule for '%s', leaving in place", name);
			continue;
		}

		w = snprintf(dst, sizeof(dst), "%s/%s/%s", dir, rule->dest, name);
		if (w < 0 || w >= (int)sizeof(dst)) {
			LOG_WARN("Destination path too long for '%s'", name);
			continue;
		}

		// rename() would silently overwrite an existing target --
		// refuse instead of destroying data.
		if (access(dst, F_OK) == 0) {
			LOG_WARN("'%s' already exists in %s/, skipping '%s'", name, rule->dest, name);
			continue;
		}

		if (!ensure_dest(dir, rule->dest)) {
			LOG_PERROR("mkdir '%s/%s'", dir, rule->dest);
			continue;
		}

		if (G_args.dry_run) {
			LOG_INFO("[dry-run] would move '%s' -> %s/", name, rule->dest);
			continue;
		}

		if (rename(src, dst) != 0) {
			LOG_PERROR("rename '%s' -> '%s'", src, dst);
			continue;
		}
		LOG_INFO("Moved '%s' -> %s/", name, rule->dest);
	}

	closedir(dp);
	return 0;
}
