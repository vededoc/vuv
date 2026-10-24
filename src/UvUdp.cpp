#define UVCLOG_LEVEL_INFO

#include <cstring>
#include "UvUdp.h"
#include "UvBufStore.h"
#include "uvcpplog.h"
#include "Loop.h"
#include <cassert>

namespace vuv {

    struct UdpSend {
        uv_udp_send_t sendReq;
        uv_buf_t buf;
    };

    struct UvUdp::UdpHandle {
        uv_udp_t handle;
        UvBufStore bufStore;
        UvBufStore sendStore;
        UvUdp* owner;
    };


    UvUdp::UvUdp(): _loop{}, _uvh{} {

    }

    UvUdp::~UvUdp() {
        if(_uvh) {
            close();
        }
    }

    int UvUdp::init() {
        if (_uvh) {
            assert(0 && "### udp handle already initialized");
            return -1;
        }
        _loop = Loop::GetUvLoop();
        _uvh = new UdpHandle;
        _uvh->handle.data = _uvh;
        _uvh->owner = this;
        int rc = uv_udp_init(_loop, &_uvh->handle);
        if (rc) {
            delete _uvh;
            _uvh = nullptr;
        }
        return rc;
    }

    int UvUdp::initEx(unsigned int flags) {
        if (_uvh) {
            assert(0 && "### udp handle already initialized");
            return -1;
        }
        _loop = Loop::GetUvLoop();
        _uvh = new UdpHandle;
        _uvh->handle.data = _uvh;
        _uvh->owner = this;
        int rc = uv_udp_init_ex(_loop, &_uvh->handle, flags);
        if (rc) {
            delete _uvh;
            _uvh = nullptr;
        }
        return rc;
    }

    void UvUdp::close() {
        if(_uvh) {
            Loop::CheckLoop(_loop);//CHECK_UV_LOOP();
            auto uvh = _uvh;
            _uvh = nullptr;
            uvh->owner = nullptr;
            uv_close((uv_handle_t*)&uvh->handle, +[](uv_handle_t* handle) {
                auto huv = (UdpHandle*)handle->data;
                delete huv;
            });
        }
    }

    int UvUdp::bind(const struct sockaddr *addr, unsigned int flags) {
        if (!_uvh || !addr) {
            return -1;
        }
        return uv_udp_bind(&_uvh->handle, addr, flags);
    }

    int UvUdp::bind(const char *ipaddr, uint16_t port, unsigned int flags) {
        if (!ipaddr) {
            return -1;
        }
        sockaddr_storage addr;
        int rc;
        if(!strchr(ipaddr, ':')) {
            rc = uv_ip4_addr(ipaddr, port, (sockaddr_in*)&addr);
        } else {
            rc = uv_ip6_addr(ipaddr, port, (sockaddr_in6*)&addr);
        }
        if(rc) {
            return rc;
        }
        return bind((const sockaddr*)&addr, flags);
    }

    int UvUdp::recvStart() {
        if (!_uvh) {
            return -1;
        }
        static auto read_alloc_cb = +[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf){
            auto uvh = (UdpHandle*)handle->data;
            buf->base = uvh->bufStore.alloc(suggested_size);
            buf->len = suggested_size;
        };

        static auto recv_cb = +[](uv_udp_t* handle, ssize_t nread,
                                  const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
            auto uvh = (UdpHandle*)handle->data;
            if(uvh->owner) {
                if(nread > 0 && uvh->owner->_recvLis) {
                    uvh->owner->_recvLis(buf->base, nread, addr, flags);
                }
            }
            uvh->bufStore.recycle(buf->base);
        };

        return uv_udp_recv_start(&_uvh->handle, read_alloc_cb, recv_cb);
    }


    int UvUdp::send(const uint8_t* buf, size_t len, const struct sockaddr* addr) {
        if (!_uvh || !addr || (!buf && len > 0)) {
            return -1;
        }
        static auto send_cb = +[](uv_udp_send_t* req, int status) {
            auto huv = (UdpHandle*)req->data;
            auto owner = huv->owner;
            huv->bufStore.recycle(((UdpSend*)req)->buf.base);
            huv->sendStore.recycle((char*)req);
            if (owner) {
                owner->procSendCallback(status);
            }
        };

        auto us = (UdpSend*)_uvh->sendStore.alloc(sizeof(UdpSend));
        us->buf.base = _uvh->bufStore.alloc(len);
        us->buf.len = len;
        if (len > 0) {
            memcpy(us->buf.base, buf, len);
        }
        us->sendReq.data = _uvh;

        auto ret = uv_udp_send(&us->sendReq, &_uvh->handle, &us->buf, 1, addr, send_cb);
        if (ret) {
            _uvh->bufStore.recycle(us->buf.base);
            _uvh->sendStore.recycle((char*)us);
            procSendCallback(ret);
        }
        return ret;
    }


    int UvUdp::send(const std::string &msg, const sockaddr *addr) {
        return send((const uint8_t*)msg.data(), msg.size(), addr);
    }

    int UvUdp::recvStop() {
        if (!_uvh) {
            return -1;
        }
        return uv_udp_recv_stop(&_uvh->handle);
    }

    void UvUdp::setOnRecvLis(UvUdp::RecvLis lis) {
        _recvLis = lis;
    }

    int UvUdp::getSockName(struct sockaddr *name, int *namelen) {
        if (!_uvh || !name || !namelen) {
            return -1;
        }
        return uv_udp_getsockname(&_uvh->handle, name, namelen);
    }

    int UvUdp::setMemberShip(const char *multicast_addr, const char *interface_addr, uv_membership membership) {
        return uv_udp_set_membership(&_uvh->handle, multicast_addr, interface_addr, membership);
    }

    int UvUdp::setMulticastLoop(int on) {
        return uv_udp_set_multicast_loop(&_uvh->handle, on);
    }

    int UvUdp::setMulticastTtl(int ttl) {
        return uv_udp_set_multicast_ttl(&_uvh->handle, ttl);
    }

    int UvUdp::setMulticastInterface(const char *interface_addr) {
        return uv_udp_set_multicast_interface(&_uvh->handle, interface_addr);
    }

    int UvUdp::setBroadcast(int on) {
        return uv_udp_set_broadcast(&_uvh->handle, on);
    }

    int UvUdp::setTtl(int ttl) {
        return uv_udp_set_ttl(&_uvh->handle, ttl);
    }

    void UvUdp::procSendCallback(int status) {
        if(_sendLis != nullptr) {
            _sendLis(status);
        }
    }

    void UvUdp::setOnSendLis(UvUdp::SendLis lis) {
        _sendLis = lis;
    }

    uv_udp_t *UvUdp::getRawHandle() {
        return _uvh ? &_uvh->handle : nullptr;
    }
}