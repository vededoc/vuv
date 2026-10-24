#pragma once

//#define UVCLOG_LEVEL_VERBOSE

#include "logdef.h"

namespace vuv {

    void LogErrPrint(const char* buf, int size);
    void LogNormalPrint(const char* buf, int size);
	void SetLogLevel(int level);

}
