
#pragma once

#include <cstdio>
#include <string.h>
#include <memory>
#include <string>
#include <mutex>

namespace vuv {

#define UVCLOG_VERBOSE 1
#define UVCLOG_DEBUG 2
#define UVCLOG_INFO 3
#define UVCLOG_WARN 4
#define UVCLOG_ERROR 5
#define UVCLOG_SILENT 6

#define MAX_UVCPP_LOGSTR_SIZE 1024

#if defined(UVCLOG_LEVEL_NONE)
#define UVCLOG_LEVEL UVCLOG_NONE
#elif defined(UVCLOG_LEVEL_ERROR)
#define UVCLOG_LEVEL UVCLOG_ERROR
#elif defined(LOG_LEVEL_WARN)
#define UVCLOG_LEVEL UVCLOG_WARN
#elif defined(UVCLOG_LEVEL_INFO)
#define UVCLOG_LEVEL UVCLOG_INFO
#elif defined(UVCLOG_LEVEL_DEBUG)
#define UVCLOG_LEVEL UVCLOG_DEBUG
#elif defined(UVCLOG_LEVEL_VERBOSE)
#define UVCLOG_LEVEL UVCLOG_VERBOSE
#endif


#ifndef UVCLOG_LEVEL
#define UVCLOG_LEVEL UVCLOG_VERBOSE
#endif

#define C_UVCLOG_ERROR "E"
#define C_UVCLOG_WARN "W"
#define C_UVCLOG_NOTICE "N"
#define C_UVCLOG_INFO "I"
#define C_UVCLOG_DEBUG "D"
#define C_UVCLOG_VERBOSE "V"

#define UVC_DEFAULT_LOG_INST (vuv::_gDefLogInst)
#ifndef UVCLOCAL_LOG_INST
#define UVCLOCAL_LOG_INST UVC_DEFAULT_LOG_INST
#endif



#define UVC_SET_LOG_LEVEL(L) UVCLOCAL_LOG_INST->level(L)
#define ___UVLIB_LOG_PRINT(LL, PUT_FN, FMTSTR, ...) do {\
    if(UVCLOG_LEVEL<=LL) { /*compile time log level check*/ \
        if(vuv::__vuv_log_level<=LL ) { \
                        char tmpbuf[MAX_UVCPP_LOGSTR_SIZE];\
						constexpr int spos=vuv::comp_checkslash(__FILE__, vuv::comp_length(__FILE__), 0); \
                        tmpbuf[MAX_UVCPP_LOGSTR_SIZE-1] = 0; \
						tmpbuf[MAX_UVCPP_LOGSTR_SIZE-2] = '~'; \
						int slen = snprintf(tmpbuf, MAX_UVCPP_LOGSTR_SIZE-2, "%s <" C_ ##LL ">[%s:%d] " FMTSTR "\n", \
                        vuv::GetLogDateTimeNow().c_str(), (const char*)(__FILE__+spos), __LINE__, ## __VA_ARGS__);\
            if(vuv::__vuv_log_level<=LL) {\
				PUT_FN(tmpbuf, slen);\
            }\
        } \
    }\
    } while(0)

#define ule(FMTSPEC, ...) ___UVLIB_LOG_PRINT(UVCLOG_ERROR, vuv::LogErrPrint, FMTSPEC, ## __VA_ARGS__)
#define ulw(FMTSPEC, ...) ___UVLIB_LOG_PRINT(UVCLOG_WARN, vuv::LogErrPrint, FMTSPEC, ## __VA_ARGS__)
#define uli(FMTSPEC, ...) ___UVLIB_LOG_PRINT(UVCLOG_INFO, vuv::LogNormalPrint, FMTSPEC, ## __VA_ARGS__)
#define uld(FMTSPEC, ...) ___UVLIB_LOG_PRINT(UVCLOG_DEBUG, vuv::LogNormalPrint, FMTSPEC, ## __VA_ARGS__)
#define ulv(FMTSPEC, ...) ___UVLIB_LOG_PRINT(UVCLOG_VERBOSE, vuv::LogNormalPrint, FMTSPEC, ## __VA_ARGS__)

    extern int __vuv_log_level;

    template<size_t N> constexpr size_t comp_length(char const (&)[N]) {
        return N-1;
    }

    constexpr int comp_checkdot(const char *str, const int p, const int n) {
        return (p <= 0) ? n : str[p] == '.' ? p : comp_checkdot(str, p - 1, n + 1);
    }

    constexpr int comp_checkslash(const char *str, const int p, const int n) {
        return p<0?0: (str[p]=='\\' || str[p]=='/')?p+1:comp_checkslash(str, p-1, n+1);
    }

    std::string GetLogTimeNow();

    void GetLogTimeNow(char *buf);

    std::string GetLogDateTimeNow();

}
