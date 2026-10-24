#pragma once

#include <functional>
#include <uv.h>

namespace vuv {

    class UvPoll {
    public:

        UvPoll();

        virtual ~UvPoll();

        int init(int fd);

        void close();

        int initSocket(int fd);

        void setOnEvent(std::function<void(int)> cb);

        int start(int events);

        void stop();

        uv_poll_t* getRawHandle();

    protected:
        std::function<void(int)> _callback;
        uv_loop_t *_loop;
        uv_poll_t *_uvh;
    };
}
