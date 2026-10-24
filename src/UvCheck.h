#pragma once

#include <functional>
#include <uv.h>

namespace vuv {
    class UvCheck {
    public:
        typedef std::function<void()> CheckCb;

        UvCheck();

        virtual ~UvCheck();

        int init();

        void close();

        int start();

        void stop();

        void setOn(CheckCb cb);

    private:
        uv_check_t *_uvh;
        uv_loop_t *_loop;
        CheckCb _cb;
    };
}
