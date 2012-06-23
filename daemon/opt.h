#ifndef OPT_H
#define OPT_H
#define OPTCOUNT_MAX 32

#include <stdio.h>

struct longopt
{
	int id;
	char shortopt;
	char *longopt;
	int arg;
	char *def;
	char *help;
	char *arghelp;
};

int opt_init(int c, char **v, struct longopt options[]);
char *opt_getval(int id);
int opt_getpos(int id);
void opt_help (FILE* out);

#endif
