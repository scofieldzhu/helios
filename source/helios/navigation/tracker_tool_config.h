/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/18
*******************************************************/
#ifndef __tracker_tool_config_h__
#define __tracker_tool_config_h__

#include "mirfak/navigation/mirfak_navigation_typedef.h"

MIRFAK_NAMESPACE_BEGIN

struct TrackerToolConfig
{
    bool isAxisCalibrated()const{
        return tip_point && long_tip_point && calib_drill && !calib_drill->isEmpty();
    }

    Vec3Opt getCalibAxisNormal()const{
        if(isAxisCalibrated()){
            return (*long_tip_point - *tip_point).normalized();
        }
        return std::nullopt;
    }

    bool isValid()const{
        bool is_ok = (type != TrackerToolType::kNone);
        is_ok = is_ok && !rom_filename.isEmpty();
        is_ok = is_ok && virtual_port != kNullToolPort;
        return  is_ok;
    }

    TrackerToolType type = TrackerToolType::kNone;
    QString name;
    QString rom_filename;
    int virtual_port = kNullToolPort;
    QStringOpt model_filepath;
    QStringOpt navi_model_filepath;
    Pt3Opt tip_point;
    Pt3Opt long_tip_point;
    QStringOpt calib_drill;
};

using TrackerToolConfigList = std::vector<TrackerToolConfig>;

NAMESPACE_END

#endif