#pragma once

#include <functional>
#include <uv.h>

namespace vuv {

    class UvAsync {
    public:
        typedef std::function<void()> AsynCb;

        UvAsync();

        virtual ~UvAsync();

        int init();

        void close();

        void setOn(AsynCb cb);

        int send();

        uv_async_t *getRawHandle();

    private:
        uv_any_handle *_uvh;
        uv_loop_t *_loop;
        AsynCb _cb;
    };

}
