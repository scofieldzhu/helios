/*******************************************************
* author: scofieldzhu
* time:2025/12/22
*******************************************************/
#include "device_administrator.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include "tracker.h"
#include "helios_navigation_typedef.h"
#include "helios/basic/log_service.h"
#include "drill_config_repository.h"

HELIOS_NAMESPACE_BEGIN

std::string QStrToStd(const QString& qstr){
    return qstr.toUtf8().toStdString();
}

DeviceAdministrator::DeviceAdministrator()
{

}

DeviceAdministrator::~DeviceAdministrator()
{

}

DeviceAdministrator& DeviceAdministrator::GetInst()
{
    static DeviceAdministrator da;
    return da;
}

Tracker* DeviceAdministrator::getDefaultTracker() const
{
    return active_tracker_.get();
}

RoboticArm* DeviceAdministrator::getDefaultRoboticArm() const
{
    return nullptr;
}

namespace{
    #define LOG_IS_NOT_POINT_STRING_VALUE_TYPE(label) SPDLOG_ERROR("The value of \"{}\" label is not point string type!", label)

    #define LOG_IS_NOT_INTEGER_VALUE_TYPE(label) SPDLOG_ERROR("The value of \"{}\" label is not integer type!", label)

    #define LOG_IS_NOT_STRING_VALUE_TYPE(label) SPDLOG_ERROR("The value of \"{}\" label is not string type!", label)

    #define LOG_IS_NOT_ARRAY_VALUE_TYPE(label) SPDLOG_ERROR("The value of \"{}\" label is not array type!", label)
    
    #define LOG_MISSING_LABEL(label_name) SPDLOG_ERROR("\"{}\" label is missing!", label_name)

    namespace LabelConstant{
        const char* kTracker = "Tracker";
        const char* kModel = "Model";
        const char* kIpAddr = "IpAddr";
        const char* kPort = "Port";
        const char* kRomFileDir = "RomFileDir";
        const char* kReferencer = "Referencer";
        const char* kProbe = "kProbe";
        const char* kLocator = "Locator";
        const char* kName = "Name";
        const char* kRomFilename = "RomFilename";
        const char* kVirtualPort = "VirtualPort";
        const char* kTools = "Tools";
        const char* kModelFilePath = "ModelFilePath";
        const char* kNaviModelFilePath = "NaviModelFilePath";
        const char* kTipPoint = "TipPoint";
        const char* kLongTipPoint = "LongTipPoint";
        const char* kType = "Type";
        const char* kCalibDrill = "CalibDrill";
    }    

    bool IsPositiveInteger(const QJsonValue& v)
    {
        if(!v.isDouble()){
            return false;
        }
        double d = v.toDouble();
        if(d != std::floor(d)){
            return false;
        }
        if(d < 0 || d > std::numeric_limits<int>::max()){
            return false;
        }
        return true;
    }

    Pt3Opt GetPointFromString(const QString& str)
    {
        auto substrs = str.split(',');
        if(substrs.size() != 3){
            return std::nullopt;
        }
        Point3 pt;
        bool is_ok = false;
        auto v = substrs[0].toDouble(&is_ok);
        if(!is_ok){
            return std::nullopt;
        }
        pt.setX(v);
        
        v = substrs[1].toDouble(&is_ok);
        if(!is_ok){
            return std::nullopt;
        }
        pt.setY(v);

        v = substrs[2].toDouble(&is_ok);
        if(!is_ok){
            return std::nullopt;
        }
        pt.setZ(v);
        return pt;
    }

    bool ParseToolConfig(QJsonObject& object, TrackerToolConfig& tool_config)
    {
        using namespace LabelConstant;

        //check Name
        if(!object.contains(kName)){
            LOG_MISSING_LABEL(kName);
            return false;
        }
        if(!object[kName].isString()){
            LOG_IS_NOT_STRING_VALUE_TYPE(kName);
            return false;
        }
        tool_config.name = object[kName].toString();

        //check RomFilename
        if(!object.contains(kRomFilename)){
            LOG_MISSING_LABEL(kRomFilename);
            return false;
        }
        if(!object[kRomFilename].isString()){
            LOG_IS_NOT_STRING_VALUE_TYPE(kRomFilename);
            return false;
        }
        tool_config.rom_filename = object[kRomFilename].toString();

        //check VirtualPort
        if(!object.contains(kVirtualPort)){
            LOG_MISSING_LABEL(kVirtualPort);
            return false;
        }
        if(!IsPositiveInteger(object[kVirtualPort])){
            LOG_IS_NOT_INTEGER_VALUE_TYPE(kVirtualPort);
            return false;
        }
        tool_config.virtual_port = object[kVirtualPort].toInt();

        //check ModelFilename
        if(object.contains(kModelFilePath)){
            if(!object[kModelFilePath].isString()){
                LOG_IS_NOT_STRING_VALUE_TYPE(kModelFilePath);
                return false;
            }
            tool_config.model_filepath = object[kModelFilePath].toString();
        }

        //check NaviModelFilename
        if(object.contains(kNaviModelFilePath)){
            if(!object[kNaviModelFilePath].isString()){
                LOG_IS_NOT_STRING_VALUE_TYPE(kNaviModelFilePath);
                return false;
            }
            tool_config.navi_model_filepath = object[kNaviModelFilePath].toString();
        }

        //check TipPoint
        if(object.contains(kTipPoint)){
            if(!object[kTipPoint].isString()){
                LOG_IS_NOT_STRING_VALUE_TYPE(kTipPoint);
                return false;
            }
            auto pt_str = object[kTipPoint].toString();
            auto pt_opt = GetPointFromString(pt_str);
            if(!pt_opt){
                LOG_IS_NOT_POINT_STRING_VALUE_TYPE(kTipPoint);
            }
            tool_config.tip_point = pt_opt;
        }

        //check LongTipPoint
        if(object.contains(kLongTipPoint)){
            if(!object[kLongTipPoint].isString()){
                LOG_IS_NOT_STRING_VALUE_TYPE(kLongTipPoint);
                return false;
            }
            auto pt_str = object[kLongTipPoint].toString();
            auto pt_opt = GetPointFromString(pt_str);
            if(!pt_opt){
                LOG_IS_NOT_POINT_STRING_VALUE_TYPE(kLongTipPoint);
            }
            tool_config.long_tip_point = pt_opt;
        }

        //check CalibDrill
        if(object.contains(kCalibDrill)){
            if(!object[kCalibDrill].isString()){
                LOG_IS_NOT_STRING_VALUE_TYPE(kCalibDrill);
                return false;
            }
            tool_config.calib_drill = object[kCalibDrill].toString();
        }

        return true;
    }

    bool ParseTrackerConfig(QJsonObject& object, TrackerConfig& out_config)
    {
        using namespace LabelConstant;

        //check Model 
        if(!object.contains(kModel)){
            LOG_MISSING_LABEL(kModel);
            return false;
        }
        if(!object[kModel].isString()){
            LOG_IS_NOT_STRING_VALUE_TYPE(kModel);
            return false;
        }
        auto type_opt = EnumTraits<TrackerDeviceType>::fromString(object[kModel].toString().toStdString());
        if(!type_opt){
            SPDLOG_ERROR("Model:\"{}\" is unrecognized!", QStrToStd(object[kModel].toString()));
            return false;
        }
        out_config.type = *type_opt;

        //check IpAddr
        if(!object.contains(kIpAddr)){
            LOG_MISSING_LABEL(kIpAddr);
            return false;
        }
        if(!object[kIpAddr].isString()){
            LOG_IS_NOT_STRING_VALUE_TYPE(kIpAddr);
            return false;
        }
        out_config.visit_host = object[kIpAddr].toString();

        //check Port
        if(!object.contains(kPort)){
            LOG_MISSING_LABEL(kPort);
            return false;
        }
        if(!IsPositiveInteger(object[kPort])){
            LOG_IS_NOT_INTEGER_VALUE_TYPE(kPort);
            return false;
        }
        out_config.port = object[kPort].toInt();

        //check RomFileDir
        if(!object.contains(kRomFileDir)){
            LOG_MISSING_LABEL(kRomFileDir);
            return false;
        }
        if(!object[kRomFileDir].isString()){
            LOG_IS_NOT_STRING_VALUE_TYPE(kRomFileDir);
        }
        out_config.rom_file_dir = object[kRomFileDir].toString();

        // check kTools
        if(!object.contains(kTools)){
            LOG_MISSING_LABEL(kRomFileDir);
            return false;
        }
        if(!object[kTools].isArray()){
            LOG_IS_NOT_ARRAY_VALUE_TYPE(kTools);
            return false;
        }
        auto tool_array = object[kTools].toArray();
        for(auto i = 0; i < tool_array.count(); ++i){
            if(!tool_array.at(i).isObject()){
                SPDLOG_ERROR("The {}th tool config is not object!", i);
                return false;
            }
            TrackerToolConfig ttc;
            auto tool_obj = tool_array.at(i).toObject();
            if(!tool_obj.contains(kType)){
                LOG_MISSING_LABEL(kType);
                return false;
            }
            if(!tool_obj[kType].isString()){
                LOG_IS_NOT_STRING_VALUE_TYPE(kType);
                return false;
            }
            auto type_str = tool_obj[kType].toString();
            auto type_opt = EnumTraits<TrackerToolType>::fromString(type_str.toStdString());
            if(!type_opt){
                SPDLOG_ERROR("Tracker tool type:\"{}\" is unrecognized!", QStrToStd(type_str));
                return false;
            }
            ttc.type = *type_opt;
            if(ttc.type == TrackerToolType::kNone){
                SPDLOG_ERROR("The field value of \"Type\" is not valid TrackerToolType value!");
                return false;
            }
            if(!ParseToolConfig(tool_obj, ttc)){
                SPDLOG_ERROR("Parse tool config with named:\"{}\"!", QStrToStd(ttc.name));
                return false;
            }
            out_config.tool_configs.emplace_back(std::move(ttc));      
        }        
        return true;
    }

    bool ParseConfigFile(const QString& abs_config_file, TrackerConfig& tc)
    {
        using namespace LabelConstant;

        QFile f(abs_config_file);
        if(!f.open(QIODevice::ExistingOnly | QIODevice::ReadOnly)){
            SPDLOG_WARN("Config file:\"{}\" not exists!", QStrToStd(abs_config_file));
            return false;
        }
        QByteArray json_data = f.readAll();
        f.close();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(json_data, &err);
        if(err.error != QJsonParseError::NoError){
            SPDLOG_WARN("Config file:\"{}\" is not well-formed json file!", QStrToStd(abs_config_file));
            return false;
        }
        auto root_obj = doc.object();
        if(!root_obj.contains(kTracker)){
            LOG_MISSING_LABEL(kTracker);
            return false;
        }
        auto tracker_obj = root_obj[kTracker].toObject();
        if(!ParseTrackerConfig(tracker_obj, tc)){
            SPDLOG_ERROR("Parse tracker config failed!");
            return false;
        }
        ////convert 'romFileDir' to absolute path
        //QDir local_dir(ResourceMgr::GetInst().getResourceDir() + "/conf");
        //out_config.tracker.rom_file_dir = local_dir.filePath(out_config.tracker.rom_file_dir);
        //if(!root_obj.contains("Robot")){
        //    qWarning() << "No 'Robot' tag found in json file:" << filename << "!\n";
        //    return false;
        //}
        //auto robot_obj = root_obj["Robot"].toObject();
        //if(!ParseRobotConfig(robot_obj, out_config.robot)){
        //    qWarning() << "Parse 'Robot' tag failed in json file:" << filename << "!\n";
        //    return false;
        //}
        return true;
    }

    bool CheckToolConfig(const TrackerToolConfig& ttc, const QString& abs_rom_dir)
    {
        if(ttc.virtual_port <= 7 || ttc.virtual_port >= 12){
            SPDLOG_ERROR("The virtual port:{} value is invalid!", ttc.virtual_port);
            return false;
        }
        auto rom_filepath = QDir(abs_rom_dir).filePath(ttc.rom_filename);
        if(!QFileInfo(rom_filepath).exists()){
            SPDLOG_ERROR("The rom file:\"{}\" not exists!", QStrToStd(ttc.rom_filename));
            return false;
        }
        if(ttc.type == TrackerToolType::kProbe){
            if(!ttc.tip_point.has_value()){
                SPDLOG_ERROR("The tip point value must exists in Probe tool[{}] config!", QStrToStd(ttc.name));
                return false;
            }
            return true;
        }
        if(ttc.type == TrackerToolType::kLocator){
            if(!ttc.calib_drill.has_value()){
                SPDLOG_ERROR("The calib drill value must exists in Locator tool[{}] config!", QStrToStd(ttc.name));
                return false;
            }else{
                if(!DrillConfigRepository::GetInst().getDrillConfig(*ttc.calib_drill)){
                    SPDLOG_ERROR("The specific calib drill:\"{}\" not exists!", QStrToStd(*ttc.calib_drill));
                    return false;
                }
            }
            if(!ttc.tip_point.has_value()){
                SPDLOG_ERROR("The tip point value must exists in Locator tool[{}] config!", QStrToStd(ttc.name));
                return false;
            }
            if(!ttc.long_tip_point.has_value()){
                SPDLOG_ERROR("The long tip point value must exists in Locator tool[{}] config!", QStrToStd(ttc.name));
                return false;
            }
            if(!ttc.model_filepath || !ttc.navi_model_filepath){
                SPDLOG_ERROR("The {} and {} value must exists in Locator tool[{}] config!", LabelConstant::kModelFilePath, LabelConstant::kNaviModelFilePath, QStrToStd(ttc.name));
                return false;
            }else{
                if(!QFileInfo(*ttc.model_filepath).exists()){
                    SPDLOG_ERROR("Model file:\"{}\" not exists!", QStrToStd(*ttc.model_filepath));
                    return false;
                }
                if(!QFileInfo(*ttc.navi_model_filepath).exists()){
                    SPDLOG_ERROR("Navi model file:\"{}\" not exists!", QStrToStd(*ttc.navi_model_filepath));
                    return false;
                }
            }
            return true;
        }
        return true;
    }

    bool CheckTrackerConfig(const TrackerConfig& tc)
    {
        if(!tc.isValid()){
            SPDLOG_ERROR("Exist some invalid field values of this Configuration!");
            return false;
        }
        QFileInfo fi(tc.rom_file_dir);
        if(!fi.exists() || !fi.isDir()){
            SPDLOG_ERROR("Rom file directory:\"{}\" is not invalid!", QStrToStd(tc.rom_file_dir));
            return false;
        }        
        for(const auto& ttc : tc.tool_configs){
            if(!CheckToolConfig(ttc, tc.rom_file_dir)){
                SPDLOG_ERROR("Check tool:\"{}\" config failed!", QStrToStd(ttc.name));
                return false;
            }
        }
        return true;
    }
}

bool DeviceAdministrator::init(const QString& config_file)
{
    TrackerConfig tc;
    if(!ParseConfigFile(config_file, tc)){
        SPDLOG_ERROR("Parse device config file:\"{}\" failed!", QStrToStd(config_file));
        return false;
    }
    //convert relative path to absolution file path in uniform handle.
    auto config_file_dir = QFileInfo(config_file).dir();
    tc.rom_file_dir = config_file_dir.filePath(tc.rom_file_dir);
    for(auto& tool_cf : tc.tool_configs){
        if(tool_cf.model_filepath){
            tool_cf.model_filepath = config_file_dir.filePath(*tool_cf.model_filepath);
        }
        if(tool_cf.navi_model_filepath){
            tool_cf.navi_model_filepath = config_file_dir.filePath(*tool_cf.navi_model_filepath);
        }
    }
    if(!CheckTrackerConfig(tc)){
        SPDLOG_ERROR("Check tracker config object failed! config file:\"{}\" failed!", QStrToStd(config_file));
        return false;
    }
    active_tracker_ = std::make_unique<Tracker>(tc);
    return true;
}

NAMESPACE_END