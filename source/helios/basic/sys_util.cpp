/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/2/9
*******************************************************/
#include "sys_util.h"
#ifdef PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif
#include <sys/timeb.h>
#include <time.h>
#include <iomanip>
#include <random>
#include <sstream>
#include <array>

using local_clock = std::chrono::steady_clock;

HELIOS_NAMESPACE_BEGIN

double GetUTCTime()
{
	double utc_time = 0.0;
	timeb current_time;
	static double scale = 1.0 / 1000.0;
	::ftime(&current_time);
	utc_time = current_time.time + scale * current_time.millitm;
	return utc_time;
}

double GetCPUTime()
{
	return static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);
}

unsigned long long GetSysTickCount()
{
#if defined(PLATFORM_WINDOWS)
	return ::GetTickCount64();
#else
	//log_fatal(systemlogger, "GetTickCount Failed!\n");
	return 0;
#endif
}

StopWatch::StopWatch()
{
    restart();
}

void StopWatch::restart()
{
    t0_ =  local_clock::now();
}

long long StopWatch::elapsedMillisecs()const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(local_clock::now() - t0_).count();
}

std::string GenUuidString()
{
    std::array<unsigned char, 16> data{};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    for(auto& b : data) {
        b = static_cast<unsigned char>(dist(gen));
    }
    // RFC 4122 version 4
    data[6] = (data[6] & 0x0F) | 0x40;
    data[8] = (data[8] & 0x3F) | 0x80;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for(size_t i = 0; i < data.size(); ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
        if(i == 3 || i == 5 || i == 7 || i == 9) {
            oss << '-';
        }
    }
    return oss.str();
}

NAMESPACE_END