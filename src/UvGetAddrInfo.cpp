#include <cassert>
#include "Loop.h"
#include "UvGetAddrInfo.h"

using namespace std;

namespace vuv {

    struct UvGetAddrInfo::AddrReq {
        uv_getaddrinfo_t req{};
        UvGetAddrInfo* owner;
    };

	UvGetAddrInfo::UvGetAddrInfo(): _loop{}, _areq{} {

	}

	UvGetAddrInfo::~UvGetAddrInfo() {
		if(_areq) {
			assert(!_areq && "### Error: addrinfo request is not free");
			close();
		}
	}

	int UvGetAddrInfo::init() {
        _loop = Loop::GetUvLoop();
		return _loop ? 0:-1;
	}

	int UvGetAddrInfo::getAddrInfo(const char *node, const char *service, const struct addrinfo *hints, Lis lis) {
        static auto callback = +[](uv_getaddrinfo_t* req, int status, struct addrinfo* res) {
            auto areq = (AddrReq*)req->data;
            if(areq->owner && areq->owner->_lis) {
                areq->owner->_lis(status, res);
            }
            uv_freeaddrinfo(res);
            delete areq;
        };
        _lis = lis;
        auto areq = new AddrReq;
        areq->owner = this;
        areq->req.data = areq;
        _areq = areq;
		return uv_getaddrinfo(_loop, &(_areq->req),callback, node, service, hints);
	}

	void UvGetAddrInfo::close() {
		if(_areq && _areq->owner) {
			_areq->owner = nullptr;
			_areq = nullptr;
		}
	}
}