#include "UvTcp.h"
#include "etc.h"
#include "uvcpplog.h"
#include <cassert>

using namespace std;

#define RAWH() ((uv_tcp_t*)getRawHandle())


namespace vuv {
	UvTcp::UvTcp() {
	}

	UvTcp::~UvTcp() {

	}


	int UvTcp::connect(const sockaddr *addr) {
        static auto cnn_cb = +[](uv_connect_t *req, int status) {
            UvTcp *ptcp = static_cast<UvTcp *>(((StreamHandle *) req->data)->owner );
            if (!status) {
                uld("connected");
            } else {
                uld("### disconnected, err_n=%d, err=%s, ename=%s", status, uv_strerror(status), uv_err_name(status));
            }

            if (ptcp) {
                if (status) {
                    if (ptcp->_uvh) {
                        ptcp->_uvh->closeUv();
                        ptcp->_uvh = nullptr;
                    }
                } else {
                    ptcp->readStart();
                }
                if (ptcp->_cnnCb) {
                    ptcp->_cnnCb(status);
                }
            }
        };

        if (!_uvh || !addr) {
            return -1;
        }
        int rc;
        _uvh->cnn.data = _uvh;
        rc = uv_tcp_connect(&_uvh->cnn, &_uvh->rawh.tcp, addr, cnn_cb);
        if (rc) {
            ulw("### tcp connect error, rc=%s", UvStrErr(rc));
        }
        return rc;
	}

	int UvTcp::connect(const std::string &addr, int port) {
        int rc;
        sockaddr_in inaddr;
        rc = uv_ip4_addr(addr.data(), port, &inaddr);
        if(rc) {
            return -1;
        }
        return connect((sockaddr*)&inaddr);
	}


	int UvTcp::bind(const struct sockaddr *addr, unsigned int flags) {
        if (!_uvh || !addr) {
            return -1;
        }
		return uv_tcp_bind(RAWH(), addr, flags);
	}

    int UvTcp::bind(int port, const char *addr) {
        if (!_uvh || !addr) {
            return -1;
        }
        struct sockaddr_in inaddr;
        int rc = uv_ip4_addr(addr, port, &inaddr);
        if (rc) {
            return rc;
        }
        return uv_tcp_bind(&_uvh->rawh.tcp, (const sockaddr *) &inaddr, 0);
    }

	int UvTcp::bindAndListen(uint16_t port, const char *ipaddr, int backlogs) {
        if (!_uvh || !ipaddr) {
            return -1;
        }
		sockaddr_in inaddr;
        int rc = uv_ip4_addr(ipaddr, port, &inaddr);
        if (rc) {
            return rc;
        }
		rc = bind((sockaddr *) &inaddr, 0);
        if (rc) {
            return rc;
        }
		return UvStream::listen(backlogs);
	}

	// vsx addition: SO_REUSEPORT 리슨. MsgWorker 워커마다 같은 포트에
	// 리슨 소켓을 열어 커널이 accept을 분산하도록 한다. init() 호출 후 사용.
	int UvTcp::bindAndListenReusePort(uint16_t port, const char *ipaddr, int backlogs) {
        if (!_uvh || !ipaddr) {
            return -1;
        }
		sockaddr_in inaddr;
		int rc = uv_ip4_addr(ipaddr, port, &inaddr);
		if (rc) return rc;
		int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
		if (fd < 0) return -errno;
		int opt = 1;
		::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
		if (::bind(fd, (sockaddr *) &inaddr, sizeof(inaddr))) {
			rc = -errno;
			::close(fd);
			return rc;
		}
		if (::listen(fd, backlogs)) {
			rc = -errno;
			::close(fd);
			return rc;
		}
		rc = uv_tcp_open(RAWH(), fd);
		if (rc) {
			::close(fd);
			return rc;
		}
		return UvStream::listen(backlogs);
	}

	int UvTcp::init() {
        int rc;
        rc = initInternal();
        if (rc) {
            return rc;
        }
        return uv_tcp_init(_loop, &_uvh->rawh.tcp);
	}

	int UvTcp::noDelay(int enable) {
        if (!_uvh) return -1;
		return uv_tcp_nodelay(RAWH(), enable);
	}

	int UvTcp::keepAlive(int enable, unsigned int delay) {
        if (!_uvh) return -1;
		return uv_tcp_keepalive(RAWH(), enable, delay);
	}

	int UvTcp::simultaneousAccepts(int enable) {
        if (!_uvh) return -1;
		return uv_tcp_simultaneous_accepts(RAWH(), enable);
	}

	int UvTcp::getSockName(struct sockaddr *name, int *namelen) {
        if (!_uvh || !name || !namelen) return -1;
		return uv_tcp_getsockname(RAWH(), name, namelen);
	}

	int UvTcp::getPeerName(struct sockaddr *name, int *namelen) {
        if (!_uvh || !name || !namelen) return -1;
		return uv_tcp_getpeername(RAWH(), name, namelen);
	}

	int UvTcp::getPeerIpPort(std::string* peerIp, int* port) {
        if (!_uvh || !peerIp || !port) {
            return -1;
        }
		int rc;
		sockaddr_storage addr;
		int nlen = sizeof(addr);
		rc = uv_tcp_getpeername(RAWH(), (sockaddr*)&addr, &nlen);
		if (rc) {
			return -1;
		}
		char tmp[128];
		if (addr.ss_family == AF_INET) {
			sockaddr_in *inaddr = (sockaddr_in*)&addr;
			*port = ntohs(inaddr->sin_port);
			uv_ip4_name(inaddr, tmp, sizeof(tmp));
			peerIp->assign(tmp);
		} else {
			sockaddr_in6 *inaddr = (sockaddr_in6*)&addr;
			*port = ntohs(inaddr->sin6_port);
			uv_ip6_name(inaddr, tmp, sizeof(tmp));
			peerIp->assign(tmp);
		}
		return 0;
	}

}
