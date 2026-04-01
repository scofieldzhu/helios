/*******************************************************
* author: scofieldzhu
* time:2025/12/23
*******************************************************/
#include "drill_config_repository.h"

HELIOS_NAMESPACE_BEGIN

DrillConfigRepository::DrillConfigRepository()
{

}

DrillConfigRepository::~DrillConfigRepository()
{

}


DrillConfigRepository& DrillConfigRepository::GetInst()
{
	static DrillConfigRepository inst;
	return inst;
}

void DrillConfigRepository::addDrillConfig(const DrillConfig& cf)
{
	if(!config_lib_.contains(cf.name)){
		config_lib_.insert(cf.name, cf);
	}
}

std::optional<DrillConfig> DrillConfigRepository::getDrillConfig(const QString& name) const
{
	if(config_lib_.contains(name)){
		return config_lib_[name];
	}
	return std::nullopt;
}

QStringList DrillConfigRepository::getAllDrillNames() const
{
	return config_lib_.keys();
}

NAMESPACE_END