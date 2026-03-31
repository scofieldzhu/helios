/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/18
*******************************************************/
#ifndef __mirfak_navigation_export_h__
#define __mirfak_navigation_export_h__

#include "mirfak/mirfak_nsp.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || defined(_WINDOWS) || defined(_WINDLL) 
    #define PLATFORM_WINDOWS  
#elif defined(__linux__)  
    #define PLATFORM_LINUX  
#elif defined(__APPLE__)  
    #define PLATFORM_MACOS  
#endif  

#if defined(PLATFORM_WINDOWS)  
    #if defined(MIRFAK_NAVIGATION_EXPORTS)  
        #define MIRFAK_NAVIGATION_API __declspec(dllexport)  
    #else  
        #define MIRFAK_NAVIGATION_API __declspec(dllimport)  
    #endif  
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)  
    #define MIRFAK_NAVIGATION_API __attribute__((visibility("default")))  
#else  
    #define MIRFAK_NAVIGATION_API  
#endif 

#endif