//
// Created by netmind on 8/1/26.
//

#include "UvSignal.h"

#include <cassert>

#include "Loop.h"
#include "uvcpplog.h"

namespace vuv {
    UvSignal::UvSignal() : _uvh(nullptr), _loop(nullptr) {
    }

    UvSignal::~UvSignal() {
        close();
    }

    int UvSignal::init() {
        int rc;
        if (_uvh) {
            assert(0 && "handle already initialized");
            return -1;
        }
        _loop = Loop::GetUvLoop();
        _uvh = new uv_signal_t;
        _uvh->data = this;

        rc = uv_signal_init(_loop, _uvh);
        if (rc) {
            delete _uvh;
            _uvh = nullptr;
        }
        return rc;
    }

    int UvSignal::start(int signum) const {
        int rc = 0;
        if(!_uvh) {
            return -1;
        }
        rc = uv_signal_start(_uvh, +[](uv_signal_t* handle, int signum) {
            auto owner = (UvSignal *) handle->data;
            if (owner && owner->_cb) {
                owner->_cb(signum);
            }
        }, signum);

        return rc;
    }

    int UvSignal::startOneShot(int signum) const {
        int rc = 0;
        if(!_uvh) {
            return -1;
        }
        rc = uv_signal_start_oneshot(_uvh, +[](uv_signal_t* handle, int signum) {
            auto owner = (UvSignal *) handle->data;
            if (owner && owner->_cb) {
                owner->_cb(signum);
            }
        }, signum);

        return rc;
    }

    void UvSignal::setOn(std::function<void(int signum)> cb) {
        _cb = cb;
    }

    void UvSignal::stop() {
        if (_uvh) {
            uv_signal_stop(_uvh);
        }
    }

    void UvSignal::close() {
        if (_uvh) {
            Loop::CheckLoop(_loop);
            uv_close((uv_handle_t *) _uvh, +[](uv_handle_t *ph) {
                delete (uv_signal_t *) ph;
            });
            _uvh = nullptr;
        }
    }
}
