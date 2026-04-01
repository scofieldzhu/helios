/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/2/27
*******************************************************/
#ifndef __resource_util_h__
#define __resource_util_h__

#include <QString>
#include "helios/appbase/helios_appbase_export.h"

class QWidget;

HELIOS_NAMESPACE_BEGIN

HELIOS_APPBASE_API void ApplyWidgetStyleSheet(QWidget* w, const QString& filepath);
HELIOS_APPBASE_API QString ReadStyleSheetFiles(const QStringList& files);

NAMESPACE_END

#endif
