/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/25
*******************************************************/
#ifndef ___ITrackerToolManager_h_
#define ___ITrackerToolManager_h_

#include <QWidget>
#include "mirfak/core/mirfak_core_typedef.h"

class ITrackerToolManager
{
public:    
    virtual ~ITrackerToolManager() = default;    
};

#define ITRACKER_TOOL_MANAGER_IID "mirfak.plugin.ITrackerToolManager/1.0"

Q_DECLARE_INTERFACE(ITrackerToolManager, ITRACKER_TOOL_MANAGER_IID)

#endif