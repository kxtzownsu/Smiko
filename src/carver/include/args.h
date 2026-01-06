/* Hannah's crappy command line parsing!
 * I literally made this when first learning C, but I'm
 * too lazy to redo the implementation for every utility,
 * so instead you get to see some of the first C I ever wrote!
 */

#ifndef __SMIKO_INCLUDE_ARGS_H
#define __SMIKO_INCLUDE_ARGS_H

extern int gargc;
extern char **gargv;

// fval("--parameter", 1) = "burger" (assuming --parameter burger was passed)
char *fval(const char *arg, const char *shorthand, int param);

// fcount("--parameter") = 3 (assuming --parameter arg0 arg1 arg2 was passed)
int fcount(const char *arg, const char *shorthand);

// fequals("--parameter"); = "burger" (assuming --parameter=burger was passed)
char *fequals(const char *arg);

// fbool("--parameter") == true (assuming --parameter was passed)
bool fbool(const char *arg, const char *shorthand);
#endif /* __SMIKO_INCLUDE_ARGS_H */