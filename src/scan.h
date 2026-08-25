/*
 * Copyright (c) 2026 Pritam
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * scan.h -- Depth-1 directory scanning
 *
 * The heart of sift: walk one directory's top level, route every
 * regular file through the rule table, and move winners into their
 * destination folder. Behavior flags come from G_args (-n, -M).
 */

#ifndef _SCAN_H_
#define _SCAN_H_

// Route the top-level files of 'dir' through the rule table and move
// matches into their rule->dest folder (created on first need).
//
// Returns 0 if the directory was scanned, -1 if it couldn't be opened
// (already logged). Individual file errors are logged and skipped --
// one bad file never aborts the run.
int scan_dir(const char *dir);

#endif  // _SCAN_H_