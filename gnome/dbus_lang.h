#ifndef DBUS_H_
#define DBUS_H_

#include <dbus/dbus.h>
#include <stdio.h>

#define DBUS_NEW_HANDLER(name,conn,msg,data) DBusHandlerResult name(DBusConnection* conn,\
		DBusMessage* msg, void* data)
#define DBUS_NEW_SIGNAL_HANDLER(interface, name) \
	DBusHandlerResult name(DBusConnection* connection,\
		DBusMessage* message, void* user_data) {\
	if (!dbus_message_is_signal(message, interface, #name))\
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
#define DBUS_NEW_METHOD_HANDLER(interface, name) \
	DBusHandlerResult name(DBusConnection* connection,\
		DBusMessage* message, void* user_data) {\
	if (!dbus_message_is_method_call(message, interface, #name))\
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
#define DBUS_END_HANDLER return DBUS_HANDLER_RESULT_HANDLED; }
#define DBUS_REPLY_END_HANDLER 	{ DBusMessage* reply = dbus_message_new_method_return(message);\
	dbus_connection_send(connection, reply, NULL); dbus_message_unref(reply); }\
	DBUS_END_HANDLER
#define __CAT(x, y) x ## y
#define _CAT(x, y) __CAT(x, y)
#define UNIQUE(name) _CAT(name,__LINE__)
#define DBUS_BUS(type) dbus_bus_get( _CAT(DBUS_BUS_,type), NULL)
#define DBUS_CALL_NOARG(bustype, name, path, iface, method) \
	DBUS_CALL(bustype, name, path, iface, method, DBUS_TYPE_INVALID)
#define DBUS_CALL(bustype, name, path, iface, method, ...) {\
	DBusMessage* UNIQUE(msg) = dbus_message_new_method_call(name, path, iface, method);\
	dbus_message_append_args(UNIQUE(msg), __VA_ARGS__, DBUS_TYPE_INVALID);\
	dbus_connection_send(DBUS_BUS(bustype), UNIQUE(msg), NULL);\
	dbus_connection_flush(DBUS_BUS(bustype));\
	dbus_message_unref(UNIQUE(msg)); }
#define DBUS_LISTEN_TO(type, interface) \
		dbus_bus_add_match(__dbus_main_connection, "type='"type"',interface='"interface"'", NULL);
#define DBUS_ADD_HANDLER(handler) \
		dbus_connection_add_filter(__dbus_main_connection, handler, NULL, dbus_free);
#define DBUS_INIT_START(bustype, name, ...) \
	int __true = 1, __false = 0;\
	int main(__VA_ARGS__) {DBusConnection *__dbus_main_connection = DBUS_BUS(bustype);\
		if (dbus_bus_request_name(__dbus_main_connection, name, DBUS_NAME_FLAG_REPLACE_EXISTING,\
			NULL) != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {\
			fputs("Not Primary Owner\n",stderr);return 1;}
#define DBUS_INIT_END \
	dbus_connection_flush(__dbus_main_connection);\
	while (dbus_connection_read_write_dispatch(__dbus_main_connection, -1));}
#define TRUEPTR &__true
#define FALSEPTR &__false
extern int __true, __false;

#endif /* DBUS_H_ */
