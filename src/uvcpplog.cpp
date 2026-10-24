#include <uv.h>
#include <cstdio>
#include <chrono>
#include <thread>
#include "uvcpplog.h"

using namespace std;

namespace vuv {

    int __vuv_log_level = UVCLOG_WARN;

    string GetLogTimeNow() {
        char buf[32];
        uv_timeval64_t ct;
        struct tm ttm;
        uv_gettimeofday(&ct);
		localtime_r(&ct.tv_sec, &ttm);
		::sprintf(buf, "%02d:%02d:%02d.%03d", ttm.tm_hour, ttm.tm_min, ttm.tm_sec, (int) (ct.tv_usec / 1000));
		return string(buf);
	}

	void GetLogTimeNow(char *buf) {
        uv_timeval64_t ct;
        struct tm ttm;
        uv_gettimeofday(&ct);
		localtime_r(&ct.tv_sec, &ttm);
		::sprintf(buf, "%02d:%02d:%02d.%03d", ttm.tm_hour, ttm.tm_min, ttm.tm_sec, (int) (ct.tv_usec / 1000));
	}

	std::string GetLogDateTimeNow() {
        char buf[64];
        uv_timeval64_t ct;
        struct tm ttm;
        uv_gettimeofday(&ct);
		localtime_r(&ct.tv_sec, &ttm);
		::sprintf(buf, "%02d/%02d-%02d:%02d:%02d.%03d", ttm.tm_mon+1, ttm.tm_mday, ttm.tm_hour, ttm.tm_min, ttm.tm_sec, (int) (ct.tv_usec / 1000));
		return string(buf);
	}

	void SetLogLevel(int level) {
        __vuv_log_level = level;
	}

    void LogNormalPrint(const char* buf, int size) {
        fwrite(buf, 1, size, stdout);
    }

    void LogErrPrint(const char* buf, int size) {
        fwrite(buf, 1, size, stderr);
    }
}

