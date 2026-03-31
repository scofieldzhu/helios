/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/8/26
*******************************************************/
#ifndef __raycasting_conf_h__
#define __raycasting_conf_h__

#include "mirfak/basic/mirfak_basic_typedef.h"
#include <QString>

struct LightnesssConf
{
    double specular = 0.0;
    double ambient = 0.0;
    double diffuse = 0.0;
};

struct CtrlPointConf
{
    double intensity = 0.0;
    mirfak::Color clr;
    double opacity = 1.0;
};

struct RaycastingConf
{
    QString name;
    std::optional<LightnesssConf> lightness;
    std::vector<CtrlPointConf> ctrl_points;
    std::optional<std::array<double, 2>> window_level;
};

using RaycastingConfGroup = std::vector<RaycastingConf>;

#endif