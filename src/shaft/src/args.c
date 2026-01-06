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

// TODO (Hannah): Add a default value?
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
	{'s', "socket", " </dev/ttyUSB0>", "Specify a socket to operate on", ARG_STRING},
	{'f', "firmware", " <gsc-fw.bin>", "Specify the input firmware", ARG_STRING},
	{'c', "certificate", " <ro-cert.bin>", "Specify an endorsement certificate", ARG_STRING},
	{'n', "nvmem", " <nvmem.bin>", "Specify an NVMEM file to write", ARG_STRING},
	{'o', "output", " <gsc-fw.rec>", "Generate a .rec file for fastboot", ARG_STRING},
	{'r', "rescue", NULL,  "Perform a GSC rescue operation", ARG_BOOLEAN},
	{'b', "bootstrap", NULL, "Perform a GSC bootstrap operation", ARG_BOOLEAN},
	{'e', "extortion", NULL, "Rescue to CFW (Cr50 RO <= 0.0.10 rescue ONLY!)", ARG_BOOLEAN},
	{'b', "bootcon", NULL, "BootCon to CFW (Cr50 RW <= 0.0.6 ONLY!)", ARG_BOOLEAN},
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

	printf("Shaft - Downgrade any Google Security Chip ™\n"
		"\n"
		"Usage: sudo %s [args]"
		"\n"
		"Shaft is a tool designed to implement Google's closed-source GSC Rescue and Bootstrap\n"
		"(aka SPIFlash) features and use them on production devices without the need for manufacturer\n"
		"interference, while also granting more than usual control of the GSC's flash banks.\n"
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
	printf("Shaft - Bootstrap any Google Security Chip ™ - Injector Version 1.2.0\n");
	printf("%s\n", BUILDSTRING);
	printf("~ Made with love by HavenOverflow <3 ~\n");
	exit(esc);
}