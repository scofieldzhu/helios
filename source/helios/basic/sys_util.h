/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/2/9
*******************************************************/
#ifndef __sys_util_h__
#define __sys_util_h__

#include <chrono>
#include "helios/basic/helios_basic_export.h"

HELIOS_NAMESPACE_BEGIN

HELIOS_BASIC_API double GetUTCTime();
HELIOS_BASIC_API double GetCPUTime();
HELIOS_BASIC_API unsigned long long GetSysTickCount();

class HELIOS_BASIC_API StopWatch
{
public:
    void restart();
    long long elapsedMillisecs()const;
    StopWatch();
    ~StopWatch() = default;    

private:
    std::chrono::steady_clock::time_point t0_;
};

HELIOS_BASIC_API std::string GenUuidString();

NAMESPACE_END

#endif