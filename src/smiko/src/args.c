/* Copyright 2025 HavenOverflow
 * Use of this code is permissible so long as the appropriate credit
 * is provided. See the LICENSE file for more info.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "common.h"
#include "version.h"

static int gargc;
static char **gargv;


/* TODO (Hannah): Finish the new argparse impl. */
enum arg_type {
	ARG_BOOLEAN,
	ARG_STRING
};

struct cmd_line_args {
	char shorthand;
	char *arg;
	char *params;
	char *desc;
	enum arg_type type;
};

const struct cmd_line_args args[] = {
	{'h', "help", NULL, "Show this help and exit", ARG_BOOLEAN},
	{'v', "verbose", NULL, "Display debug logs on stdout", ARG_BOOLEAN},
	{'V', "version", NULL, "Show the utility version and exit", ARG_BOOLEAN},
	{'H', "headerinfo", " <gsc-fw.bin>", "Display the values of a GSC firmware's header", ARG_STRING},
	{'i', "sysinfo", " <gsc-fw.bin>", "Display the system info of the target GSC", ARG_BOOLEAN},
	{'d', "dump", " <gsc-fw.bin>", "Dump signature, key, info, and fuse data from a GSC firmware's header", ARG_STRING},
	{'f', "flash", " <gsc-fw.bin>", "Program a provided GSC firmware to the target GSC's flash", ARG_STRING},
	{'r', "verify", " <gsc-fw.bin>", "Perform a full image verification of the target GSC firmware", ARG_STRING},
	{'s', "section", " [RO/RW/BOTH]", "Optionally specify a section in the target GSC firmware", ARG_STRING},
	{'c', "console", " <tty path>", "Open a console on the provided TTY", ARG_STRING},
	{'p', "reset", " <immediate/post/update>", "Reset the target GSC", ARG_STRING},
	{'S', "suzyq", NULL, "Transfer to the target GSC over USB", ARG_BOOLEAN},
	{'t', "trunks", NULL, "Transfer to the target GSC over trunksd", ARG_BOOLEAN}
};

int fbool(char *arg)
{
	char buf[strlen(arg) + 3];
	int i, argoff = -1;
	/* Find our argument definition. */
	for (i = 0; i < ARRAY_LEN(args); ++i) {
		if (!strcmp(arg, args[i].arg)) {
			argoff = i;
			break;
		}
	}

	strcpy(&buf[2], arg);
	buf[0] = buf[1] = '-';

	for (i = 0; i < gargc; ++i) {
		if (!strcmp(buf, gargv[i]))
			return i;
	}

	if (argoff != -1) {
		buf[1] = args[argoff].shorthand;
		buf[2] = 0; // NULL
		for (i = 0; i < gargc; ++i) {
			if (!strcmp(buf, gargv[i])) 
				return i;
		}
	}

	return 0;
}

char *fval(char *arg)
{
	int offset = fbool(arg);
	if (!offset)
		return NULL;
	
	if (gargv[offset + 1][0] == '-')
		return NULL;
	
	return gargv[offset + 1];
}

void parse_args(int argc, char **argv)
{
	gargc = argc;
	gargv = argv;
}

void show_info(int esc)
{
	int i;

	printf("Smiko - Smite Management by hIjacKing the cr50\n"
		"\n"
		"Usage: sudo %s [args]"
		"\n"
		"Smiko is a tool designed to serve as a driver and interface for existing\n"
		"Google Security Chips. It contains lots of useful debugging and information\n"
		"tools, and serves as a more advanced and flexible alternative to GSCTool.\n"
		"\n"
		"Arguments:\n",
		gargv[0]);
	for (i = 0; i < ARRAY_LEN(args); ++i) {
		printf("-%c, --%s%s: %s\n", args[i].shorthand, args[i].arg,
				(args[i].params) ? args[i].params : "", args[i].desc);
	}

	exit(esc);
}

void show_ver(int esc)
{
	printf("Smiko - Smite Management by hIjacKing the cr50 - Injector Version 1.1.0\n");
	printf("%s\n", BUILDSTRING);
	printf("~ Made with love by HavenOverflow <3 ~\n");
	exit(esc);
}