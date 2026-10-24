#include <sstream>
#include "Loop.h"
#include <cassert>
#include <unordered_map>

#include "etc.h"
#include "uvcpplog.h"

using namespace std;

namespace vuv {

    thread_local Loop *gLoop=nullptr;
    thread_local bool gRunning=false;

	Loop::Loop(): _loop{}, _loopCreated{} {
	}

	Loop::~Loop() {
	}


	void Loop::Open(uv_loop_t *loop) {
		if(gLoop) {
			assert(0 && "Event loop is not idle in this thread");
			return;
		}
		auto inst = new Loop();
		if(!loop) {
            inst->_loop = new uv_loop_t;
            inst->_loopCreated = true;
			loop = inst->_loop;
			uv_loop_init(inst->_loop);
		}
        inst->_loop = loop;
        gLoop = inst;
	}

	void Loop::Close() {
		if(gLoop) {
			auto inst = gLoop;

			if (inst->_loopCreated) {
				if (inst->_loop) {
                    int rc = uv_loop_close(inst->_loop);
                    // close 콜백이 아직 처리되지 않아 EBUSY가 될 수 있다.
                    // 루프를 강제로 delete하면 살아있는 핸들이 dangling pointer가 되므로
                    // non-blocking으로 close 콜백을 소진한 뒤 재시도한다.
                    for (int i = 0; i < 3 && rc == UV_EBUSY; ++i) {
                        uv_run(inst->_loop, UV_RUN_NOWAIT);
                        rc = uv_loop_close(inst->_loop);
                    }
                    if (rc) {
                        ulw("### uv_loop_close failed, rc=%s", UvStrErr(rc));
                        // live handle이 남아 있으면 loop 메모리를 해제하지 않는다.
                        gLoop = nullptr;
                        delete inst;
                        return;
                    }
					delete inst->_loop;
                    inst->_loop = nullptr;
				}
                inst->_loopCreated = false;
			}
			delete inst;
            gLoop = nullptr;
		}
	}

	Loop *Loop::Get() {
		return gLoop;
	}


    uv_loop_t *Loop::GetUvLoop() {
        return gLoop ? gLoop->_loop : nullptr;
    }

    void Loop::CheckLoop(uv_loop_t *loop) {
        assert(loop && loop == GetUvLoop());
    }

    uv_loop_t *Loop::getLoop() {
		return _loop;
	}

	int Loop::Run(uv_run_mode mode) {
		assert(gLoop);
		gRunning = true;
		int rc = uv_run(gLoop->_loop, mode);
		gRunning = false;
		return rc;
	}

    bool Loop::IsRunning() {
        return gRunning;
    }

    int64_t Loop::tick() {
        return uv_now(_loop);
    }

	unordered_map<std::string, int> Loop::getEventHandles() {
		std::unordered_map<std::string, int> handles;
		uv_walk(_loop, +[](uv_handle_t* handle, void* arg) {
			auto hm = (std::unordered_map<std::string, int>*)arg;
			auto type = uv_handle_type_name(handle->type);
			auto itr = hm->find(type);
			if (itr != hm->end()) {
				itr->second++;
			} else {
				hm->insert(std::make_pair(uv_handle_type_name(handle->type), 1));
			}
		}, &handles);
		return handles;
	}
}