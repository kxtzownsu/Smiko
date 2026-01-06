/* Hannah's crappy command line parsing!
 * I literally made this when first learning C, but I'm
 * too lazy to redo the implementation for every utility,
 * so instead you get to see some of the first C I ever wrote!
 */

#include <cstring>
#include <string>

#include "args.hh"

int gargc;
char **gargv;

// fval("--parameter", 1) = "burger" (assuming --parameter burger was passed)
std::string fval(std::string arg, std::string shorthand, int param)
{
	for (int i = 0; i < gargc; i++) {
		if (!strcmp(gargv[i], arg.c_str()) || !strcmp(gargv[i], shorthand.c_str())) 
			return gargv[i + param];
	}

	return "";
}

// fcount("--parameter") = 3 (assuming --parameter arg0 arg1 arg2 was passed)
int fcount(std::string arg, std::string shorthand)
{
	int i;
	for (i = 0; i < gargc; i++) {
		if (!strcmp(gargv[i], arg.c_str()) || !strcmp(gargv[i], shorthand.c_str()))
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
std::string fequals(std::string arg)
{
	for (int i = 0; i < gargc; i++) {
		if (!memcmp(gargv[i], arg.c_str(), strlen(arg.c_str()) - 1)) 
			return gargv[i] + strlen(arg.c_str()) + 1;
	}

	return "";
}

// fbool("--parameter") == true (assuming --parameter was passed)
bool fbool(std::string arg, std::string shorthand)
{
	for (int i = 0; i < gargc; i++) {
		if (!strcmp(gargv[i], arg.c_str()) || !strcmp(gargv[i], shorthand.c_str())) 
			return true;
	}

	return false;
}