/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/18
*******************************************************/
#ifndef __tool_tracking_data_h__
#define __tool_tracking_data_h__

#include "helios/navigation/helios_navigation_typedef.h"

HELIOS_NAMESPACE_BEGIN

struct ToolTrackingData
{
    void reset()
    {        
        tool_err = kInvalidToolError;
        status = TrackerToolStatus::kMissing;
        is_valid = false;
        tool_pos.zero();
        tool_axis_normal = std::nullopt;
        unregistrated_tool_pos.zero();
        unregistrated_tool_axis_normal = std::nullopt;
    }
    int id = -1;
    bool loaded = false;
    double tool_err = kInvalidToolError;
    TrackerToolStatus status = TrackerToolStatus::kMissing;
    bool is_valid = false;
    Point3 tool_pos; 
    std::optional<Vec3> tool_axis_normal; 
    Point3 unregistrated_tool_pos;    
    std::optional<Vec3> unregistrated_tool_axis_normal; 
};

using ToolTrackingDataList = std::vector<ToolTrackingData>;

NAMESPACE_END

#endif
