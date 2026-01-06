/* Hannah's crappy command line parsing!
 * I literally made this when first learning C, but I'm
 * too lazy to redo the implementation for every utility,
 * so instead you get to see some of the first C I ever wrote!
 */

extern int gargc;
extern char **gargv;

// fval("--parameter", 1) = "burger" (assuming --parameter burger was passed)
std::string fval(std::string arg, std::string shorthand, int param);

// fcount("--parameter") = 3 (assuming --parameter arg0 arg1 arg2 was passed)
int fcount(std::string arg, std::string shorthand);

// fequals("--parameter"); = "burger" (assuming --parameter=burger was passed)
std::string fequals(std::string arg);

// fbool("--parameter") == true (assuming --parameter was passed)
bool fbool(std::string arg, std::string shorthand);