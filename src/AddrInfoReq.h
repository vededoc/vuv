#pragma once
#include <list>
#include <uv.h>
#include "Loop.h"

namespace vuv {
	struct AddrInfoReq {
		std::list<AddrInfoReq>::iterator itr;
		uv_getaddrinfo_t req;
		void* obj;
		Loop* ctx;
	};

	struct NameInfoReq {
		std::list<NameInfoReq>::iterator itr;
		uv_getnameinfo_t req;
		void* obj;
		Loop* ctx;
	};
}