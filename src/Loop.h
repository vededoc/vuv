#pragma once
#include <unordered_map>
#include <string>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <uv.h>

namespace vuv {

	class Loop {
	public:
		static void Open(uv_loop_t *loop=nullptr);

		static void Close();

		static Loop *Get();

		static int Run(uv_run_mode mode=UV_RUN_DEFAULT);

        static uv_loop_t *GetUvLoop();

        static bool IsRunning();

		uv_loop_t *getLoop();

        int64_t tick();

		std::unordered_map<std::string, int> getEventHandles();

        static void CheckLoop(uv_loop_t* loop);

	private:

		bool _loopCreated;
		uv_loop_t *_loop;

		Loop();
		virtual ~Loop();
	};
}
