#include <cassert>
#include "UvPoll.h"
#include "uvcpplog.h"
#include "Loop.h"

namespace vuv {

    UvPoll::UvPoll() : _loop{}, _uvh{} {

    }

    UvPoll::~UvPoll() {
        close();
    }

    int UvPoll::init(int fd) {
        if (_uvh) {
            assert(0);
            return -1;
        }
        _loop = Loop::GetUvLoop();
        _uvh = new uv_poll_t;
        _uvh->data = this;
        return uv_poll_init(_loop, _uvh, fd);
    }

    int UvPoll::initSocket(int fd) {
        if (_uvh) {
            assert(0);
            return -1;
        }
        _loop = Loop::GetUvLoop();
        _uvh = new uv_poll_t;
        _uvh->data = this;
        return uv_poll_init_socket(_loop, _uvh, fd);
    }

    void UvPoll::close() {
        if (_uvh) {
            _uvh->data = nullptr;
            Loop::CheckLoop(_loop);//CHECK_UV_LOOP();
            uv_close((uv_handle_t *) _uvh, +[](uv_handle_t *handle) {
                delete (uv_poll_t *) handle;
            });
            _uvh = nullptr;
        }
    }

    int UvPoll::start(int events) {
        if (!_uvh) {
            return -1;
        }
        auto rc = uv_poll_start(_uvh, events, +[](uv_poll_t *uvh, int status, int events) {
            auto owner = (UvPoll *) uvh->data;
            if (owner && owner->_callback) {
                owner->_callback(events);
            }
        });
        return rc;
    }

    void UvPoll::stop() {
        if (_uvh) {
            uv_poll_stop(_uvh);
        }
    }

    uv_poll_t *UvPoll::getRawHandle() {
        return _uvh;
    }

    void UvPoll::setOnEvent(std::function<void(int)> cb) {
        _callback = cb;
    }
}