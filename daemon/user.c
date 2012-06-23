#include "daemon.h"

#include <grp.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>

#define pam_do(pamh, funct, fail,...) { \
	int err = funct(__VA_ARGS__);\
	if (err) {\
		lprintf(LOG_ERR, #funct": %s", pam_strerror(pamh, err));\
		fail;}}

pam_handle_t *pamh;

int session_setup(struct passwd* pw, struct xserver *server)
{
	static struct pam_conv conv = { misc_conv, NULL };

	pam_do(pamh, pam_start, return EPAM, "lightsm", pw->pw_name, &conv, &pamh);

	pam_do(pamh, pam_set_item, return EPAM, pamh, PAM_TTY, server->displaydev);
	pam_do(pamh, pam_set_item, return EPAM, pamh, PAM_RUSER, "root");
	pam_do(pamh, pam_set_item, return EPAM, pamh, PAM_XDISPLAY, server->displayname);
	pam_do(pamh, pam_acct_mgmt, return EPAM, pamh, 0);

	if (setgid(pw->pw_gid) || initgroups(pw->pw_name, pw->pw_gid)) {
		lprintf(LOG_ERR, "failed to set groups for user `%s': %m\n",
				pw->pw_name);
		return errno;
	}

	pam_do(pamh, pam_setcred, return EPAM, pamh, PAM_ESTABLISH_CRED);
	pam_do(pamh, pam_open_session,
			pam_setcred(pamh, PAM_DELETE_CRED); return EPAM, pamh, 0);
	lprintf(LOG_INFO, "pam session started");
	return 0;
}

int session_switch_user(struct passwd* pass, struct xserver *server)
{
	char dbus_address[PATH_MAX], buf[PATH_MAX], **env = pam_getenvlist(pamh);
	FILE *file;
	if (setuid(pass->pw_uid)) {
		lprintf(LOG_ERR, "failed to switch user\n");
		return EPERM;
	}
	clearenv();
	setenv("USER", pass->pw_name, 1);
	setenv("LOGNAME", pass->pw_name, 1);
	setenv("HOME", pass->pw_dir, 1);
	setenv("SHELL", pass->pw_shell, 1);
	setenv("DISPLAY", server->displayname, 1);
	snprintf(dbus_address, PATH_MAX,
			"DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/%s/dbus/user_bus_socket",
			pass->pw_name);
	putenv(dbus_address);
	putenv(server->cmdenv);
	while (*env)
		putenv(*(env++));

	file = popen("/bin/bash -l -c env", "r");
	while (fgets(buf, PATH_MAX - 1, file)) {
		*(strchrnul(buf, '\n')) = '\0';
		putenv(strdup(buf));
	}
	pclose(file);
	return 0;
}

void session_cleanup()
{
	if (pamh) {
		pam_close_session(pamh, 0);
		pam_setcred(pamh, PAM_DELETE_CRED);
	}
}
