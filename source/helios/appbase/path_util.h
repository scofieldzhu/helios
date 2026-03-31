/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/25
*******************************************************/
#ifndef __path_util_h__
#define __path_util_h__

#include "mirfak/appbase/mirfak_appbase_export.h"
#include "mirfak/appbase/mirfak_appbase_typedef.h"

MIRFAK_NAMESPACE_BEGIN

namespace path_util
{
    MIRFAK_APPBASE_API bool RemoveTree(const QString &path, bool remove_self);
    MIRFAK_APPBASE_API QStringList SearchDirFiles(const QString& dir, const QStringList& required_extension_names);
    MIRFAK_APPBASE_API void SetDirHidden(const QString& dir);
};

NAMESPACE_END

#endif 
