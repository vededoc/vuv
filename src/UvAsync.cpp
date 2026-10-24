#define UVCLOG_LEVEL_INFO

#include <cassert>
#include "UvAsync.h"
#include "uvcpplog.h"
#include "Loop.h"
namespace vuv {

    UvAsync::UvAsync(): _loop(nullptr), _uvh(nullptr) {
//        uli("ctor uv async, ptr=%p", this);
    }

    UvAsync::~UvAsync() {
        if(_uvh) {
            close();
        }
    }

    int UvAsync::init() {
        uld("uv async init, ptr=%p", this);
        int rc;
        if(_uvh) {
            assert(0);
            return -1;
        }
        _loop = Loop::GetUvLoop();
        _uvh = new uv_any_handle;
        _uvh->handle.data = this;
        rc = uv_async_init(_loop, &_uvh->async, +[](uv_async_t *handle) {
            auto hs = (UvAsync*) handle->data;
            if (hs && hs->_cb) {
                hs->_cb();
            }
        });

        return rc;
    }

    void UvAsync::close() {
        if(_uvh) {
            Loop::CheckLoop(_loop);
            uv_close((uv_handle_t*)_uvh, +[](uv_handle_t* handle) {
                delete (uv_any_handle *)handle;
            });
            _uvh = nullptr;
        }
    }

    int UvAsync::send() {
        if(!_uvh) {
            return -1;
        }
        return uv_async_send(&_uvh->async);
    }

    void UvAsync::setOn(UvAsync::AsynCb cb) {
        _cb = cb;
    }

    uv_async_t *UvAsync::getRawHandle() {
        return (uv_async_t *)_uvh;
    }

}