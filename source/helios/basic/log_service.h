/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/7
*******************************************************/
#ifndef __log_service_h__
#define __log_service_h__

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include "mirfak/basic/mirfak_basic_export.h"

#define LOG_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            SPDLOG_ERROR("Assertion failed: {}", message); \
            throw std::runtime_error(message); \
        } \
    } while (false)

class vtkObject;

MIRFAK_NAMESPACE_BEGIN

MIRFAK_BASIC_API void InitLogger(std::shared_ptr<spdlog::logger> logger);

MIRFAK_BASIC_API std::string VtkObjToLogStr(vtkObject* obj, int indent_level = 0);

NAMESPACE_END

#endif