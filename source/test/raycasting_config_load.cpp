/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/8/26
*******************************************************/
#include "raycasting_config_load.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QFile>
#include "helios/basic/log_service.h"

using namespace helios;

bool ParseCtrlPointConfigTag(QJsonArray arry, CtrlPointConf& result)
{
    if(arry.size() != 5){
        return false;
    }
    std::array<double, 5> values;
    for(auto i = 0; i < arry.size(); ++i){
        if(!arry[i].isDouble()){
            return false;
        }
        values[i] = arry[i].toDouble();
    }
    result.intensity = values[0];
    result.opacity = values[1];
    result.clr.setRedR(values[2]);
    result.clr.setGreenR(values[3]);
    result.clr.setBlueR(values[4]);    
    return true;
}

bool ParseConfigTag(QJsonObject obj, RaycastingConf& result)
{
    if(!obj.contains("lookuptable") || !obj["lookuptable"].isArray()){
        return false;
    }
    auto ctrl_point_confs = obj["lookuptable"].toArray();
    for(auto i = 0; i < ctrl_point_confs.size(); ++i){
        if(!ctrl_point_confs[i].isArray()){
            return false;
        }
        auto item = ctrl_point_confs[i].toArray();
        CtrlPointConf cpc;
        if(!ParseCtrlPointConfigTag(item, cpc)){
            return false;
        }
        result.ctrl_points.emplace_back(std::move(cpc));        
    }
    if(obj.contains("lightness")){
        if(!obj["lightness"].isObject()){
            return false;
        }
        auto l_obj = obj["lightness"].toObject();
        if(!l_obj.contains("S") || !l_obj.contains("A") || !l_obj.contains("D")){
            return false;
        }
        LightnesssConf lc;
        lc.specular = l_obj["S"].toDouble();
        lc.ambient = l_obj["A"].toDouble();
        lc.diffuse = l_obj["D"].toDouble();
        result.lightness = lc;        
    }
    if(obj.contains("wlw")){
        if(!obj["wlw"].isArray()){
            return false;
        }
        auto val_arry = obj["wlw"].toArray();
        if(val_arry.size() != 2){
            return false;
        }
        std::array<double, 2> wlw;
        wlw[0] = val_arry[0].toDouble();
        wlw[1] = val_arry[1].toDouble();
        result.window_level = wlw;
    }
    return true;
}

RaycastingConfGroup LoadConfigFile(const QString &filepath)
{
    RaycastingConfGroup gp;
    QFileInfo fi(filepath);
    if(!fi.exists()){
        SPDLOG_ERROR("File:{} not exists!", filepath.toUtf8().constData());
        return gp;
    }
    QFile f(filepath);
    if(!f.open(QIODevice::ReadOnly | QIODevice::ExistingOnly)){
        SPDLOG_ERROR("Open File:{} for reading failed!", filepath.toUtf8().constData());
        return gp;
    }
    QByteArray all_data = f.readAll();
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(all_data, &err);
    if(err.error != QJsonParseError::NoError){
        SPDLOG_ERROR("File:{} is not well-formed json format!", filepath.toUtf8().constData());
        return gp;
    }
    auto root = doc.object();
    for(auto key : root.keys()){
        RaycastingConf conf;
        conf.name = key;
        if(!ParseConfigTag(root[key].toObject(), conf)){
            SPDLOG_ERROR("Parse raycasting config item with name {} failed!", key.toUtf8().constData());
            return {};
        }
        gp.emplace_back(std::move(conf));
    }
    SPDLOG_TRACE("Parse raycasting config file:{} successfully!", filepath.toUtf8().constData());
    return gp;
}
