#include "daemon.h"

#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>

int systemd_notify_ready(const char *address) {
	int fd = -1, r = -EINVAL;
	struct sockaddr_un sockaddr = { AF_UNIX };
	if (address && ( *address=='/' || *address=='@' )) {
		strncpy(sockaddr.sun_path, address, sizeof(sockaddr.sun_path));
		if (sockaddr.sun_path[0]=='@')
			sockaddr.sun_path[0] = 0;

		if ((fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0)) >= 0 &&
				sendto(fd, "READY=1", 7, MSG_NOSIGNAL, &sockaddr,
						sizeof(struct sockaddr_un)) >= 0)
			r=0;
		else
			r=-errno;
		close(fd);
	}
	return r;
}
