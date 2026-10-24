#include <cassert>
#include "Loop.h"

#include "UvIdle.h"
#include "uvcpplog.h"

namespace vuv {

	UvIdle::UvIdle(): _loop(nullptr), _uvh(nullptr), _cb([](){}), _started(false) {

	}

	UvIdle::~UvIdle() {
//		assert(!_uvh);
		if(_uvh) {
			deinit();
		}
	}

	int UvIdle::init() {
		if(_uvh) {
			assert(0);
			return -1;
		}

		_loop =  Loop::GetUvLoop();
		_uvh = (uv_idle_t*)malloc(sizeof(uv_idle_t)) ;
		uv_idle_init(_loop, _uvh);
		_uvh->data = this;
		return 0;
	}

	void UvIdle::deinit() {
		if(_uvh) {
			_uvh->data = nullptr;
			uv_close((uv_handle_t*)_uvh, +[](uv_handle_t* handle) {
				//uli("on close");
				free(handle);
			});
			_uvh = nullptr;
		}
	}

	void UvIdle::start() {
		if(_started) {
			return;
		}
        if (!_uvh) {
            return;
        }

		// uv_idle_start 는 항상 성공
		uv_idle_start(_uvh, +[](uv_idle_t* idle) {
			if(idle->data) {
				((UvIdle *) idle->data)->_cb();
			}
		});
		_started = true;
		return;
	}

	void UvIdle::stop() {
		if(_uvh) {
			uv_idle_stop(_uvh);
			_started = false;
		}
	}

	void UvIdle::setOn(std::function<void()> cb) {
		_cb = cb;
	}

	uv_handle_t *UvIdle::getHandle() {
		return (uv_handle_t *)_uvh;
	}
}