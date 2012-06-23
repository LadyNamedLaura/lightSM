#include "daemon.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/vt.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

int prepare_x(struct xserver* server, struct passwd* pw) {
	struct stat statbuf;
	struct vt_stat vtstat;
	char lockfile[PATH_MAX], logfile[PATH_MAX] = "", dpi[PATH_MAX] = "",
			vtstring[6] = "";
	int fd;

	server->displaynum = -1;
	do {
		snprintf(lockfile, PATH_MAX, "/tmp/.X%d-lock", ++server->displaynum);
	} while (stat(lockfile, &statbuf) == 0);
	snprintf(server->displayname, 5, ":%d", server->displaynum);

	if ((fd = open("/dev/console", O_WRONLY | O_NOCTTY, 0))>= 0) {
		if (ioctl(fd, VT_GETSTATE, &vtstat) >= 0)
			for (server->ttynum = strtol(opt_getval('f'), NULL, 10);
					vtstat.v_state & 1 << server->ttynum; server->ttynum++)
				;
		close(fd);
	}
	if(server->ttynum) {
		snprintf(vtstring, 6, "vt%d", server->ttynum);
		snprintf(server->displaydev, 12, "/dev/tty%d", server->ttynum);
	}

	if (stat(opt_getval('X'), &statbuf))
		return errno;
	else if (!(statbuf.st_mode & S_IXOTH))
		return ENOEXEC;
	else if (!(statbuf.st_mode & S_ISUID))
		snprintf(logfile, PATH_MAX, "-logfile %s/.Xorg.0.log", pw->pw_dir);

	lprintf(LOG_INFO, "taking display device");
	if(chown(server->displaydev, pw->pw_uid, pw->pw_gid))
		lprintf(LOG_WARNING, "failed to take ownership, %m");
	if (strcmp(opt_getval('d'), "auto"))
		snprintf(dpi, PATH_MAX, "-dpi %s", opt_getval('d'));
	snprintf(server->cmdenv, PATH_MAX, "LIGHTSM_X11_SERVER_CMDLINE=%s %s %s %s %s %s",
			opt_getval('X'), server->displayname, vtstring, logfile, dpi, opt_getval('o'));
	lprintf(LOG_INFO, "starting X server with: \"%s\"", server->cmdenv);
	return 0;
}
