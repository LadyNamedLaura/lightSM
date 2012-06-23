#include "daemon.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <pwd.h>
#include <sys/wait.h>

struct longopt options[] = {
	{ 'h', 'h', "help",     0, NULL, "print this help message", NULL},
	{ 'V', 'V', "version",  0, NULL, "print " NAME "'s version number", NULL},
	{ 'v', 'v', "verbose",  0, NULL, "verbose output or logging", NULL},
	{ 'q', 'q', "quiet",    0, NULL, "only log warnings and errors", NULL},
	{ 'u', 'u', "user",     1, NULL, "the user to launch " NAME " for", "USER"},
	{ 'U',  0 , "unit",     1, "default.target", "which systemd unit to launch", "UNIT"},
	{ 'f',  0 , "first-vt", 1,  "1", "the first vt to use", "N"},
	{ 'd', 'd', "dpi",      1, "auto", "the dpy to set", "DPI"},
	{ 'X', 'X', "X-server", 1, "/usr/bin/Xorg", "path to X-server to use", "PATH"},
	{ 'o', 'o', "xopt",     1, "-nolisten tcp -noreset", "additional options to pass to Xorg", "OPT"},
	{  0 ,  0 , NULL, 0, NULL, NULL, NULL}
};

int log_level = LOG_DEBUG;

int lprintf(int prio, const char* fmt, ...) {
	va_list ap;
	if (log_level < prio)
		return -1;
	if (fmt == NULL)
		return 0;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	putc('\n', stderr);
	va_end(ap);
	return 0;
}

static void do_help(int argc, char** argv, FILE* out, int exitval) {
	fprintf(out, "Usage: %s --user=USER [options]\n\n", argv[0]);
	fprintf(out, "Options:\n");
	opt_help(out);
	exit(exitval);
}

void do_kill(pid_t pid, int timeout) {
	int info, status;
	status = waitpid(pid, &info, WNOHANG);
	if (status == -1 || WIFEXITED(info) || WIFSIGNALED(info)) /* child does not exist */
		return;
	kill(pid, SIGTERM);
	sleep(timeout);
	status = waitpid(pid, &info, WNOHANG);
	if (status == -1 || WIFEXITED(info) || WIFSIGNALED(info)) /* child does not exist */
		return;
	kill(pid, SIGKILL);
	waitpid(pid, NULL, 0);
}

int main(int argc, char **argv) {
	sigset_t set;
	siginfo_t info;
	pid_t pid;
	int ret;
	struct xserver server;
	char unit[PATH_MAX];
	struct passwd *pw;

	if (opt_init(argc, argv, options) || opt_getpos('u') == 0)
		do_help(argc, argv, stderr, EINVAL);
	if (opt_getpos('h'))
		do_help(argc, argv, stdout, ESUCCESS);
	if (opt_getpos('V')) {
		puts(NAME " version " VERSION "\n");
		return ESUCCESS;
	}
	if (getuid()) {
		lprintf(LOG_ERR, NAME": can only be run by root");
		return EPERM;
	}

	if (opt_getpos('q') - opt_getpos('v') > 0)
		log_level = LOG_WARNING;
	else if (opt_getpos('q') - opt_getpos('v') < 0)
		log_level = LOG_DEBUG;

	pw = getpwnam(opt_getval('u'));
	if (!pw) {
		lprintf(LOG_ERR, "unknown username: %s", opt_getval('u'));
		return EINVAL;
	}

	prepare_x(&server, pw);
	ret = session_setup(pw, &server);
	if (ret)
		return ret;

	pid = fork();
	switch (pid) {
	case 0:
		session_switch_user(pw, &server);

		sprintf(unit, "--unit=%s", opt_getval('U'));
		kill(getppid(), SIGUSR1);
		execl(SYSTEMD_PATH, SYSTEMD_PATH, "--user", unit, NULL);
		return ENOEXEC;
	case -1:
		lprintf(LOG_ERR, "cannot fork user shell: %m");
		session_cleanup();
		return ECHILD;
	default:
		sigfillset(&set);
		sigprocmask(SIG_BLOCK, &set, NULL);
wait:
		switch (sigwaitinfo(&set, &info)) {
		case SIGCHLD:
			if (info.si_pid != pid)
				goto wait;
			lprintf(LOG_ERR, "systemd died");
			session_cleanup();
			return ret;
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			do_kill(pid, 10);
			session_cleanup();
			return ret;
		case SIGUSR1:
			systemd_notify_ready(getenv("NOTIFY_SOCKET"));
		default:
			goto wait;
		}
	}
}
