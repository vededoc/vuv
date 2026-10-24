#pragma once

#include <memory>
#include <utility>
#include "UvStream.h"
#include <string>

namespace vuv {
	class UvTcp : public UvStream {
	public:

		UvTcp();

		virtual ~UvTcp();

		int init();

		int noDelay(int enable);

		int keepAlive(int enable, unsigned int delay);

		int simultaneousAccepts(int enable);

		int getSockName(struct sockaddr* name, int* namelen);

		int getPeerName(struct sockaddr* name, int* namelen);

		int connect(const std::string &addr, int port);

		int connect(const sockaddr *addr);

		int bind(const struct sockaddr *addr, unsigned int flags);

        int bind(int port, const char *addr);

		int bindAndListen(uint16_t port, const char *ipaddr, int backlogs = 128);

		// vsx addition: SO_REUSEPORT 리슨 (멀티워커 accept 분산용). init() 후 호출.
		int bindAndListenReusePort(uint16_t port, const char *ipaddr, int backlogs = 128);

		int getPeerIpPort(std::string* peerIp, int *port);

	};
}