#include <cassert>
#include "uvcpplog.h"
#include "UvCheck.h"
#include "Loop.h"


using namespace std;

namespace vuv {

    UvCheck::UvCheck(): _uvh{}, _loop{} {

    }

    int UvCheck::init() {
        _loop = Loop::GetUvLoop();
        _uvh = new uv_check_t;
        _uvh->data = this;

        return uv_check_init(_loop, _uvh);
    }

    void UvCheck::close() {
        if (_uvh) {
            Loop::CheckLoop(_loop);
            uv_close((uv_handle_t *) _uvh, +[](uv_handle_t *handle) {
                delete (uv_check_t *) handle;
            });
            _uvh = nullptr;
        }
    }

    int UvCheck::start() {
        if (!_uvh) {
            assert(0 && "### not initialized");
            return -1;
        }
        auto rc = uv_check_start(_uvh, +[](uv_check_t *handle) {
            auto hs = (UvCheck *) handle->data;
            if (hs && hs->_cb) {
                hs->_cb();
            }
        });
        return rc;
    }

    void UvCheck::stop() {
        if (_uvh) {
            uv_check_stop(_uvh);
        }
    }

    void UvCheck::setOn(UvCheck::CheckCb cb) {
        _cb = cb;
    }

    UvCheck::~UvCheck() {
        if (_uvh) {
            close();
        }
    }



}
