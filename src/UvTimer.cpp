#include <cassert>
#include "Loop.h"
#include "UvTimer.h"
#include "uvcpplog.h"

namespace vuv {

    UvTimer::UvTimer() : _uvh{}, _loop{} {

    }

    UvTimer::~UvTimer() {
        if (_uvh) {
            close();
        }
    }

    int UvTimer::init() {
        if (_uvh) {
            assert(0);
            return -1;
        }
        _loop = Loop::GetUvLoop();
        _uvh = new uv_timer_t;
        _uvh->data = this;
        auto rc = uv_timer_init(_loop, _uvh);
        if (rc) {
            delete _uvh;
            _uvh = nullptr;
        }
        return rc;
    }

    int UvTimer::start(int64_t time_out, int64_t interval) {
        if(!_uvh) {
            return -1;
        }
        if (!_cb) {
            ulw("### no timer callback");
            return -1;
        }
        return uv_timer_start(_uvh, +[](uv_timer_t *ptimer) {
            auto owner = (UvTimer *) ptimer->data;
            if (owner && owner->_cb) {
                owner->_cb();
            }
        }, time_out, interval);
    }

    void UvTimer::stop() {
        if(_uvh) {
            uv_timer_stop(_uvh);
        }
    }

    void UvTimer::close() {
        if (_uvh) {
            Loop::CheckLoop(_loop);//CHECK_UV_LOOP();
            uv_close((uv_handle_t *) _uvh, +[](uv_handle_t *ph) {
                delete (uv_timer_t *) ph;
            });
            _uvh = nullptr;
        }
    }

    void UvTimer::setOn(std::function<void()> cb) {
        _cb = cb;
    }

    uv_timer_t *UvTimer::getRawHandle() {
        return _uvh;
    }

    void UvTimer::setTag(const std::string &tag) {
        _tag = tag;
    }

    int UvTimer::again() {
        if (!_uvh) {
            return -1;
        }
        return uv_timer_again(_uvh);
    }

    void UvTimer::setRepeat(uint64_t repeat) {
        if (!_uvh) {
            return;
        }
        uv_timer_set_repeat(_uvh, repeat);
        if(repeat == 0) {
            stop();
        }
    }

    uint64_t UvTimer::getRepeat() {
        return _uvh ? uv_timer_get_repeat(_uvh) : 0;
    }
}