#include <uv.h>
#include "etc.h"

namespace vuv {
    /**
     * uv_strerror() 함수는 메모리 leak 오류 발생하기 때문에
     * wrapper 함수 만듦.
     * @param err
     * @return
     */
    const char* UvStrErr(int err) {
        static char buf[128];
        // uv_stderror() 함수는 쓰지 말 것. memory leak 발생함.
        return uv_strerror_r(err, buf, sizeof(buf));
    }

}