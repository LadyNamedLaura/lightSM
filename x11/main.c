#define _GNU_SOURCE

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
	char *ptrs[30], *opt, *cmdline = getenv("LIGHTSM_X11_SERVER_CMDLINE");
	int count = 0;
	sigset_t set;
	struct timespec time = { 60L, 0L };
	if (!cmdline)
		return EINVAL;
	switch (fork()) {
	case 0:
		for (opt = strtok(cmdline, " "); opt; opt = strtok(NULL, " "))
			ptrs[count++] = strdup(opt);
		ptrs[count]=NULL;
		signal(SIGUSR1, SIG_IGN );
		execv(ptrs[0], ptrs);
	case -1:
		return ENOEXEC;
	default:
		sigfillset(&set);
		sigprocmask(SIG_BLOCK, &set, NULL);
		switch(sigtimedwait(&set, NULL, &time)) {
		case SIGUSR1:
			return 0;
		case -1:
			return errno;
		default:
			return 1;
		}
	}
}
