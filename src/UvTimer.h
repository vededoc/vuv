#pragma once

#include <uv.h>
#include <functional>
#include <string>
namespace vuv {

    class UvTimer {
    public:
        UvTimer();
        UvTimer(const UvTimer&) = delete;
        UvTimer& operator=(const UvTimer&) = delete;

        virtual ~UvTimer();

        int init();

        int start(int64_t time_out, int64_t interval);

        void stop();

        void setOn(std::function<void()> cb);

        void close();

        void setTag(const std::string& tag);

        uv_timer_t* getRawHandle();

        int again();

        void setRepeat(uint64_t repeat);

        uint64_t getRepeat();

    private:
        std:: string _tag;
        uv_loop_t *_loop;
        std::function<void()> _cb;
        uv_timer_t* _uvh;
    };
}

