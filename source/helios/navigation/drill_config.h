/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/9/15
*******************************************************/
#ifndef __drill_config_h__
#define __drill_config_h__

#include <QString>
#include "mirfak/mirfak_nsp.h"

MIRFAK_NAMESPACE_BEGIN

struct DrillConfig
{
    void reset(){
        name.clear();
        length = 0.0;
        diameter = 0.0;
        model_filepath.clear();
    }
    explicit operator bool()const{
        return (!name.isEmpty()) && length > 0.0;
    }
    bool operator==(const DrillConfig& rhs)const{
        return name == rhs.name;
    }
    bool operator!=(const DrillConfig& rhs)const{
        return name != rhs.name;
    }
    QString name;
    double length = 0.0;
    double diameter = 0.0;  
    QString model_filepath;
};

NAMESPACE_END

#endif