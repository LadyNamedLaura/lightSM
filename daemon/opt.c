#define _GNU_SOURCE
#include "opt.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

union _longopt {
	struct longopt public;
	struct {
		int id;
		char shortopt;
		char *longopt;
		int arg;
		char *def;
		char *help;
		char *arghelp;
		char *val;
		int pos;
	};
};
union _longopt opts[OPTCOUNT_MAX] = { };
int optcount = 0;

int opt_init(int argc, char **argv, struct longopt o[]) {
	struct option options[OPTCOUNT_MAX];
	char shortopts[OPTCOUNT_MAX * 3];
	int shortoptcount = 0;
	for (optcount = 0; o[optcount].id != 0; optcount++) {
		opts[optcount].public = o[optcount];
		if (opts[optcount].def)
			opts[optcount].val = strdup(opts[optcount].def);
		struct option opt = { opts[optcount].longopt, opts[optcount].arg, NULL,
				0 };
		options[optcount] = opt;
		if (opts[optcount].shortopt) {
			shortopts[shortoptcount++] = opts[optcount].shortopt;
			if (opts[optcount].arg)
				shortopts[shortoptcount++] = ':';
			if (opts[optcount].arg == 2)
				shortopts[shortoptcount++] = ':';
		}
	}
	while (1) {
		int index = 0;
		char shortopt = getopt_long(argc, argv, shortopts, options, &index);
		if (shortopt == '?')
			return -1;
		if (shortopt == -1)
			return 0;
		if (shortopt)
			for (index = 0;
					index < optcount && opts[index].shortopt != shortopt;
					index++)
				;
		opts[index].pos = optind;
		if (opts[index].arg && optarg) {
			if (opts[index].val)
				free(opts[index].val);
			opts[index].val = strdup(optarg);
		}
	}
}

int _opt_getindex(int id) {
	for (int i = 0; i < optcount; i++)
		if (opts[i].id == id)
			return i;
	return -1;
}

char *opt_getval(int id) {
	return opts[_opt_getindex(id)].val;
}

int opt_getpos(int id) {
	return opts[_opt_getindex(id)].pos;
}

void opt_help(FILE* out) {
	for (int i = 0; i < optcount; i++) {
		char longopt[23];
		snprintf(longopt, 23, "--%s                     ", opts[i].longopt);
		if (opts[i].arg == 1)
			snprintf(longopt, 23, "--%s=%s                  ", opts[i].longopt,
					opts[i].arghelp);
		else if (opts[i].arg == 2)
			snprintf(longopt, 23, "--%s[=%s]                ", opts[i].longopt,
					opts[i].arghelp);
		if (opts[i].shortopt)
			fprintf(out, "  -%c, %s %s\n", opts[i].shortopt, longopt,
					opts[i].help);
		else
			fprintf(out, "      %s %s\n", longopt, opts[i].help);
		if (opts[i].def)
			fprintf(out, "                               Default: \"%s\"\n",
					opts[i].def);
	}
}
