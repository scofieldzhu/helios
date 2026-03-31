/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/2/27
*******************************************************/
#include "resource_util.h"
#include <QFile>
#include <QWidget>
#include "mirfak/appbase/mirfak_appbase_typedef.h"
#include "mirfak/basic/log_service.h"

MIRFAK_NAMESPACE_BEGIN

namespace{
	QString ReadSingleStyleSheetFile(const QString& filepath)
	{
		QFile file(filepath);
		file.open(QFile::ReadOnly);
		QString stylesheet_str = QLatin1String(file.readAll());
		file.close();
		return stylesheet_str;
	}
}

void ApplyWidgetStyleSheet(QWidget* w, const QString& filepath)
{
	QString stylesheet_str = ReadSingleStyleSheetFile(filepath);
	if(w && !stylesheet_str.isEmpty()){
		w->setStyleSheet(stylesheet_str);
		//SPDLOG_DEBUG("filepath:{} str:{}", mirfak::QStrToLogStr(filepath), mirfak::QStrToLogStr(stylesheet_str));
	}	
}

QString ReadStyleSheetFiles(const QStringList& files)
{
	QStringList ss_strs;
	for(const auto& fp : files){
		ss_strs.push_back(ReadSingleStyleSheetFile(fp));
	}
	return ss_strs.join(" ");
}

NAMESPACE_END