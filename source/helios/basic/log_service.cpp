/******************************************************** 
* author: scofieldzhu
* time:2026/1/7
*******************************************************/
#include "log_service.h"
#include <vtkObject.h>

HELIOS_NAMESPACE_BEGIN

namespace{
	std::shared_ptr<spdlog::logger> stLogger;
}

void InitLogger(std::shared_ptr<spdlog::logger> logger)
{
	stLogger = std::move(logger);
	spdlog::set_default_logger(stLogger);
}

std::string VtkObjToLogStr(vtkObject* obj, int indent_level /*= 0*/)
{
	std::ostringstream oss;
	obj->PrintSelf(oss, vtkIndent(indent_level));
	return oss.str();
}

NAMESPACE_END