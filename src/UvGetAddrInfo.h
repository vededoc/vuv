#pragma once

#include <functional>
#include "AddrInfoReq.h"

namespace vuv {

	class UvGetAddrInfo {
		friend class Loop;
	public:
		typedef std::function<void(int, struct addrinfo*)> Lis;

		UvGetAddrInfo();

		virtual ~UvGetAddrInfo();

		int init();
		int getAddrInfo(const char* node, const char* service, const struct addrinfo* hints, Lis lis);
		void close();
	private:
        struct AddrReq;
		Lis _lis;
        AddrReq* _areq;
        uv_loop_t * _loop;
	};
}