/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/18
*******************************************************/
#ifndef __tracker_config_h__
#define __tracker_config_h__

#include "mirfak/navigation/tracker_tool_config.h"

MIRFAK_NAMESPACE_BEGIN

struct TrackerConfig
{    
    TrackerToolConfigList getToolConfigs(TrackerToolType t)const{
        TrackerToolConfigList result_configs;
        for(const auto& conf : tool_configs){
            if(conf.type == t){
                result_configs.push_back(conf);
            }
        }
        return result_configs;
    }

    int getNumberOfTool(TrackerToolType t)const{
        int cnt = 0;
        for(const auto& conf : tool_configs){
            if(t == TrackerToolType::kNone || conf.type == t){
                ++cnt;
            }
        }
        return cnt;
    }

    std::optional<TrackerToolConfig> getToolConfig(int id)const{
        if(id >= 0 && id < tool_configs.size()){
            return tool_configs[id];
        }
        return std::nullopt;
    }

    std::optional<TrackerToolConfig> getToolConfig(const QString& name)const{
        for(const auto& conf : tool_configs){
            if(conf.name == name){
                return conf;
            }
        }
        return std::nullopt;
    }

    std::optional<TrackerToolConfig> getToolConfig(TrackerToolType t, int id)const{
        int index = 0;
        for(const auto& conf : tool_configs){
            if(conf.type == t && (id < 0 || id == index++)){
                return conf;
            }
        }
        return std::nullopt;
    }

    bool isValid()const{
        bool is_ok = type != TrackerDeviceType::kNone;
        is_ok = is_ok && !visit_host.isEmpty();
        is_ok = is_ok && port > 0;
        is_ok = is_ok && !rom_file_dir.isEmpty();
        for(const auto& t : tool_configs){
            is_ok = is_ok && t.isValid();
        }
        return is_ok;
    }

    TrackerDeviceType type = TrackerDeviceType::kNone;
    QString visit_host;
    int port = kNullToolPort;
    QString rom_file_dir;
    TrackerToolConfigList tool_configs;
};

NAMESPACE_END

#endif