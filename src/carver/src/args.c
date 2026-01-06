/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#include <stdbool.h>
#include <string.h>

#include "args.h"

int gargc;
char **gargv;

// fval("--parameter", 1) = "burger" (assuming --parameter burger was passed)
char *fval(const char *arg, const char *shorthand, int param)
{
	for (int i = 0; i < gargc; i++) {
		if (!strcmp(gargv[i], arg) || !strcmp(gargv[i], shorthand)) 
			return gargv[i + param];
	}

	return "";
}

// fcount("--parameter") = 3 (assuming --parameter arg0 arg1 arg2 was passed)
int fcount(const char *arg, const char *shorthand)
{
	int i;
	for (i = 0; i < gargc; i++) {
		if (!strcmp(gargv[i], arg) || !strcmp(gargv[i], shorthand))
			break;
	}

	if (i >= gargc)
		return -1;

	int count;

	for (count = 0; i < gargc; ++count) {
		if (gargv[i][0] == '-')
			break;
	}

	return count;
}

// fequals("--parameter"); = "burger" (assuming --parameter=burger was passed)
char *fequals(const char *arg)
{
	for (int i = 0; i < gargc; i++) {
		if (!memcmp(gargv[i], arg, strlen(arg) - 1)) 
			return gargv[i] + strlen(arg) + 1;
	}

	return "";
}

// fbool("--parameter") == true (assuming --parameter was passed)
bool fbool(const char *arg, const char *shorthand)
{
	for (int i = 0; i < gargc; i++) {
		if (!strcmp(gargv[i], arg) || !strcmp(gargv[i], shorthand)) 
			return true;
	}

	return false;
}