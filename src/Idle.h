#pragma once

#include <functional>
#include <uv.h>

namespace vuv {
	class Idle {
	public:
		Idle();

		int init();

		void start();

		void stop();

		void deinit();

		void setOn(std::function<void()> cb);

		uv_handle_t *getHandle();

		virtual ~Idle();

	private:
		uv_loop_t *_loop;
		uv_idle_t* _uvh;
		bool _started;
		std::function<void()> _cb;
	};
}