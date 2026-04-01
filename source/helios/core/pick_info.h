/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/2/2
*******************************************************/
#ifndef __pick_info_h__
#define __pick_info_h__

#include "helios/core/helios_core_typedef.h"

class vtkProp;

HELIOS_NAMESPACE_BEGIN

struct PickInfo
{
    explicit operator bool()const{
        return sprite != nullptr;
    }
    void reset(){
        prop = nullptr;
        position = Point3();
        normal = Point3();
        vector = Point3();
        distance = 0.0;
        sprite = nullptr;
        scene = nullptr;
    }
    vtkProp* prop = nullptr;
    Point3 position;
    Point3 normal;
    Vec3 vector;
    double distance = 0.0;
    Sprite* sprite = nullptr;
    Scene* scene = nullptr;
};

using PickInfoList = std::vector<PickInfo>;

NAMESPACE_END

#endif