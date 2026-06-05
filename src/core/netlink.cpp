#include "core/netlink.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <unistd.h>

#include "handlers/logger.hpp"

using namespace sconnman;

NetlinkCtx::NetlinkCtx(int buf_size) {
	this->buf_size = buf_size;
	this->buf = (char *)malloc(this->buf_size);
	if (!buf) {
		Logger::log(ERR, "malloc: %s(%i)", strerror(errno), errno);
		throw std::runtime_error(strerror(errno));
	}

	this->sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (this->sock < 0) {
		Logger::log(ERR, "socket: %s(%i)", strerror(errno), errno);
		free(this->buf);
		throw std::runtime_error(strerror(errno));
	}

	this->seq = 0;
	this->pid = getpid();

	struct sockaddr_nl addr;
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = this->pid;

	if (bind(this->sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		Logger::log(ERR, "bind: %s(%i)", strerror(errno), errno);
		close(this->sock);
		free(this->buf);
		throw std::runtime_error(strerror(errno));
	}
}

NetlinkCtx::~NetlinkCtx() {
	if (this->sock >= 0) {
		close(this->sock);
		this->sock = -1;
	}
	if (this->buf) {
		free(this->buf);
		this->buf = nullptr;
	}
	this->buf_size = 0;
	this->seq = 0;
	this->pid = 0;
}

int NetlinkCtx::receive() {
	if (!_on_recv_callback) {
		Logger::log(ERR, "receive: %s", "ReceiveHandler not setted");
		return -1;
	}

	ssize_t len = -1;
	while ((len = recv(this->sock, this->buf, this->buf_size, 0)) > 0) {
		int res = _on_recv_callback(this->buf, len);
		if (res <= 0) {
			return res;
		}
	}

	if (len < 0) {
		Logger::log(ERR, "recv: %s", strerror(errno));
		return -1;
	}
	return 0;
}

/**
 * @brief 
 * set `ReceiveHandler` function returns -1 on error, 0 on success, and 1 while in progress
 * @param f 
 */
void NetlinkCtx::onReceiveCallback(ReceiveHandler f) {
	_on_recv_callback = f;
}
