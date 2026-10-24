#pragma once

#include <uv.h>
#include <functional>
#include <string>

namespace vuv {

    class UvSpawn {
    public:
        typedef std::function<void(const char* buf, size_t size)> ConsoleOutCb;

        UvSpawn();

        int run(const std::vector<std::string>& args);
        int processKill(int signum);
        void close();
        void setOnConsoleOut(ConsoleOutCb sout, ConsoleOutCb eout);
        void setOnExit(std::function<void(int)> cb);
    private:
        ConsoleOutCb _stdoutCb;
        ConsoleOutCb _stderrCb;
        std::function<void(int)> _exitCb;
        uv_loop_t* _loop;
        uv_pipe_t* _spipe;
        uv_pipe_t* _epipe;
        uv_process_t* _proc;
    };
}
