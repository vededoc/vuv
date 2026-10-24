#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include "UvIdle.h"

namespace vuv {

	class Immediate {
	public:
		typedef std::function<void()> ImmdCb;

		Immediate();

		uint32_t setImmediate(ImmdCb cb);

		void clearImmediate(uint32_t hImmd);

		void clearAll();

		void stop();

		virtual ~Immediate();

	private:
		struct Immd {
			uint32_t hImmd;
			ImmdCb cb;
		};
		ImmdCb _cb;
		UvIdle _idle;
		uint32_t _hSeed;
		std::list<Immd> _cbs;
		std::list<Immd> _tmpCbs;
	};
}
