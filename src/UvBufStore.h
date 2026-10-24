//
// Created by vededoc on 23. 10. 21.
//

#pragma once


#include <cstdint>
#include <cstddef>

namespace vuv {
    class UvBufStore {
    public:
        UvBufStore();

        virtual ~UvBufStore();

        char *alloc(size_t dsize);

        void recycle(char *buf);

        void setMaxBufSize(size_t cap);

    private:
        struct Buf;
        struct Buf *_head{};
        int _listCnt{};
        size_t _maxCapacity;
#ifndef NDEBUG
        int _allocCnt{};
#endif

        void freeBufs();
    };
}