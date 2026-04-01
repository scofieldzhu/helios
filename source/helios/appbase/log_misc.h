/******************************************************** 
* author: scofieldzhu
* time:2026/1/5
*******************************************************/
#ifndef __log_misc_h__
#define __log_misc_h__

#include <QString>
#include "helios/appbase/helios_appbase_export.h"

HELIOS_NAMESPACE_BEGIN

HELIOS_APPBASE_API std::string QStrToLogStr(const QString& qstr);

NAMESPACE_END

#endif