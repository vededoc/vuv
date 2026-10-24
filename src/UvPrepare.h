#pragma once

#include <uv.h>
#include <functional>

namespace vuv {

    class UvPrepare {
    public:

        UvPrepare();

        int init();

        void close();

        int start(std::function<void()> cb);

        void stop();

        virtual ~UvPrepare();

        uv_prepare_t *getRawHandle();

    private:
        uv_loop_t *_loop;
        uv_prepare_t* _uvh;
        std::function<void()> _callback;
    };
}
