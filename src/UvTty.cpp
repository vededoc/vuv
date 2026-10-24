#include <cassert>
#include "UvTty.h"

namespace vuv {

    UvTty::UvTty(uv_file fd, int readable) : fd_(), readable_(readable) {

    }

    UvTty::~UvTty() {

    }

    int UvTty::init() {
        auto rc = initInternal();
        if (rc) {
            assert(0);
            return rc;
        }
        return uv_tty_init(_loop, &_uvh->rawh.tty, fd_, readable_);
    }

    int UvTty::setMode(uv_tty_mode_t mode) {
        return uv_tty_set_mode(&_uvh->rawh.tty, mode);
    }

    int UvTty::resetMode(void) {
        return uv_tty_reset_mode();
    }

    int UvTty::getWinSize(int *width, int *height) {
        return uv_tty_get_winsize(&_uvh->rawh.tty, width, height);
    }
}