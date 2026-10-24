#include <cassert>
#include "Immediate.h"
#include "uvcpplog.h"

namespace vuv {

	Immediate::Immediate() : _hSeed(1) {

	}

	Immediate::~Immediate() {
		assert(!_idle.getHandle() && "### immediate exists");
		stop();
	}

	uint32_t Immediate::setImmediate(Immediate::ImmdCb cb) {
		if(!_idle.getHandle()) {
			// uv idle 이 초기화 안되어 있으면 생성한다.
			_idle.setOn([this]() {
//				uli("on immd");

				// callback 큐에 더이상 리스트가 없으면 idle 을 stop 한다.
				// uv idle 은 stop 안 해주면 event loop 가 계속 깨어나서 돌게한다.(헛돌음)
				if(!_cbs.size()) {
//					uli("stop immd");
					_idle.stop();
				}

				// callback을 호출해 주기 위해 콜백 리스트를 잘라서 가져온다.
				// 콜백에 의해 새로 등록되는 콜백은 다음 이벤트 루프에서 실행된다.
				_tmpCbs = std::move(_cbs);
				assert(_cbs.size() == 0);
				for(auto &immd: _tmpCbs) {
                    if (immd.cb) {
                        immd.cb();
                    }
				}
			});

			_idle.init();
		}

		_cbs.emplace_back();
		auto &immd = _cbs.back();
		immd.cb = cb;
		immd.hImmd = _hSeed;

		_hSeed+=2;

		_idle.start();
		return immd.hImmd;
	}


	void Immediate::clearImmediate(uint32_t hImmd) {
		for(auto itr = _cbs.begin(); itr != _cbs.end(); itr++) {
			if(itr->hImmd == hImmd) {
				itr->cb = +[](){};
				break;
			}
		}
	}

	void Immediate::clearAll() {
		for(auto &immd: _tmpCbs) {
			immd.cb = +[](){};
		}
		_cbs.clear();
		_idle.stop();
	}

	void Immediate::stop() {
		clearAll();
		_idle.deinit();
	}
}