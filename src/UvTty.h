#pragma once

#include "UvStream.h"

namespace vuv {

    class UvTty : public UvStream {
    public:
        // 0: STDIN, 1: STDOUT, 2: STDERR
        UvTty(uv_file fd=0, int readable=1);

        virtual ~UvTty();

        int init() ;

        int setMode(uv_tty_mode_t mode);

        int resetMode(void);

        int getWinSize(int *width, int *height);

    private:
        int fd_;
        int readable_;

    };

}
