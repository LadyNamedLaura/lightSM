#include "dbus_lang.h"

#define BUSNAME "org.gnome.SessionManager"
#define INTERFACE BUSNAME
#define ENDSESSIONDIALOG "org.gnome.SessionManager.EndSessionDialog"

void shell_open_end_session_dialog(int type, int timestamp, int timeout) {
	void *array = NULL;
	DBUS_CALL(SESSION, "org.gnome.Shell",
		"/org/gnome/SessionManager/EndSessionDialog",
		ENDSESSIONDIALOG, "Open", DBUS_TYPE_UINT32, &type,
		DBUS_TYPE_UINT32, &timestamp, DBUS_TYPE_UINT32, &timeout,
		DBUS_TYPE_ARRAY, DBUS_TYPE_OBJECT_PATH, &array, 0);
}

DBUS_NEW_METHOD_HANDLER(INTERFACE, Logout)
	int mode;
	dbus_message_get_args(message, NULL, DBUS_TYPE_UINT32, &mode,
		DBUS_TYPE_INVALID);
	if (mode == 0)
		shell_open_end_session_dialog(0, 0, 60);
	else
		DBUS_CALL_NOARG(SESSION, "org.freedesktop.systemd1", 
			"/org/freedesktop/systemd1",
			"org.freedesktop.systemd1.Manager", "Exit");
DBUS_REPLY_END_HANDLER

DBUS_NEW_METHOD_HANDLER(INTERFACE, Shutdown)
	shell_open_end_session_dialog(1, 0, 60);
DBUS_REPLY_END_HANDLER

DBUS_NEW_SIGNAL_HANDLER(ENDSESSIONDIALOG, ConfirmedLogout)
	DBUS_CALL_NOARG(SESSION, "org.freedesktop.systemd1",
		"/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager",
		"Exit" );
DBUS_END_HANDLER

DBUS_NEW_SIGNAL_HANDLER(ENDSESSIONDIALOG, ConfirmedShutdown)
	DBUS_CALL(SYSTEM, "org.freedesktop.login1", "/org/freedesktop/login1",
		"org.freedesktop.login1.Manager", "PowerOff", DBUS_TYPE_BOOLEAN,
		TRUEPTR);
DBUS_END_HANDLER

DBUS_NEW_SIGNAL_HANDLER(ENDSESSIONDIALOG, ConfirmedReboot)
	DBUS_CALL(SYSTEM, "org.freedesktop.login1", "/org/freedesktop/login1",
		"org.freedesktop.login1.Manager", "Reboot", DBUS_TYPE_BOOLEAN,
		TRUEPTR);
DBUS_END_HANDLER

DBUS_INIT_START(SESSION, BUSNAME, )
	DBUS_LISTEN_TO("signal",ENDSESSIONDIALOG);
	DBUS_LISTEN_TO("method",INTERFACE);
	DBUS_ADD_HANDLER(Shutdown);
	DBUS_ADD_HANDLER(Logout);
	DBUS_ADD_HANDLER(ConfirmedLogout);
	DBUS_ADD_HANDLER(ConfirmedShutdown);
	DBUS_ADD_HANDLER(ConfirmedReboot);
DBUS_INIT_END
