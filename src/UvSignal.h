//
// Created by vededoc on 8/1/26.
//

#pragma once
#include <functional>
#include <uv.h>

namespace vuv {
    class UvSignal {
    public:
        UvSignal();
        virtual ~UvSignal();

        int init();
        int start(int signum) const;
        int startOneShot(int signum) const;
        void setOn(std::function<void(int signum)> cb);
        void stop();
        void close();

    private:
        uv_signal_t* _uvh;
        uv_loop_t* _loop;
        std::function<void(int signum)> _cb;
    };
}
