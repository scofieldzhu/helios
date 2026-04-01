/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/23
*******************************************************/
#ifndef __drill_config_repository_h__
#define __drill_config_repository_h__

#include <QMap>
#include <optional>
#include "helios/navigation/drill_config.h"
#include "helios/navigation/helios_navigation_export.h"

HELIOS_NAMESPACE_BEGIN

class HELIOS_NAVIGATION_API DrillConfigRepository
{
public:
	static DrillConfigRepository& GetInst();
	void addDrillConfig(const DrillConfig& cf);
	std::optional<DrillConfig> getDrillConfig(const QString& name)const;
	QStringList getAllDrillNames()const;
	DrillConfigRepository(const DrillConfigRepository&) = delete;
	DrillConfigRepository& operator=(const DrillConfigRepository&) = delete;

private:
	DrillConfigRepository();
	~DrillConfigRepository();	
	QMap<QString, DrillConfig> config_lib_;
};


NAMESPACE_END

#endif