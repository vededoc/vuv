#include "UvPrepare.h"
#include "Loop.h"

using namespace std;

namespace vuv {

    UvPrepare::UvPrepare(): _uvh{}, _loop{} {

    }

    int UvPrepare::init() {
        _loop = Loop::GetUvLoop();
        _uvh = new uv_prepare_t ;
        _uvh->data = this;
        return uv_prepare_init(Loop::GetUvLoop(), _uvh);
    }

    void UvPrepare::close() {
        if(_uvh) {
            Loop::CheckLoop(_loop);
            uv_close((uv_handle_t*)_uvh, +[](uv_handle_t* handle) {
                delete (uv_prepare_t *)handle;
            });
            _uvh = nullptr;
        }
    }

    UvPrepare::~UvPrepare() {
        if(_uvh) {
            close();
        }
    }

    void UvPrepare::stop() {
        if (_uvh) {
            uv_prepare_stop(_uvh);
        }
    }

    int UvPrepare::start(function<void()> cb) {
        if (!_uvh) {
            return -1;
        }
        _callback = cb;
        auto rc = uv_prepare_start(_uvh, +[](uv_prepare_t *uvh) {
            auto hs = (UvPrepare *) uvh->data;
            if (hs && hs->_callback) {
                hs->_callback();
            }
        });
        return rc;
    }

    uv_prepare_t *UvPrepare::getRawHandle() {
        return _uvh;
    }
}