/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/5
*******************************************************/
#ifndef __log_misc_h__
#define __log_misc_h__

#include <QString>
#include "mirfak/appbase/mirfak_appbase_export.h"

MIRFAK_NAMESPACE_BEGIN

MIRFAK_APPBASE_API std::string QStrToLogStr(const QString& qstr);

NAMESPACE_END

#endif