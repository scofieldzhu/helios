/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/25
*******************************************************/
#ifndef __path_util_h__
#define __path_util_h__

#include "helios/appbase/helios_appbase_export.h"
#include "helios/appbase/helios_appbase_typedef.h"

HELIOS_NAMESPACE_BEGIN

namespace path_util
{
    HELIOS_APPBASE_API bool RemoveTree(const QString &path, bool remove_self);
    HELIOS_APPBASE_API QStringList SearchDirFiles(const QString& dir, const QStringList& required_extension_names);
    HELIOS_APPBASE_API void SetDirHidden(const QString& dir);
};

NAMESPACE_END

#endif 
