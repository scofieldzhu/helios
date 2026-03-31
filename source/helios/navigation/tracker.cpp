/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/2/6
*******************************************************/
#include "tracker.h"
#define WIN32_LEAN_AND_MEAN 
#include <winsock2.h>  
#include <windows.h> 
#include <iphlpapi.h>  
#include <icmpapi.h>      
#include <ws2tcpip.h>  
#include <vtkTrackerTool.h>
#include <vtkHybridVegaTracker.h>
#include <AIMTracker/vtkAIMTracker.h>
#include <vtkARMDTracker.h>
#include <QDir>
#include <QTimer>
#include "mirfak/basic/sys_util.h"
#include "mirfak/basic/log_service.h"
#pragma comment(lib, "iphlpapi.lib")  
#pragma comment(lib, "ws2_32.lib")  

MIRFAK_NAMESPACE_BEGIN

bool TestReachableIp(const std::string& ip)
{
    HANDLE icmp_handle = IcmpCreateFile();
    if(icmp_handle == INVALID_HANDLE_VALUE){
        SPDLOG_WARN("Create Icmp handle failed! error code:{}.", GetLastError());
        return false;
    }
    struct in_addr addr;
    int result = InetPtonA(AF_INET, ip.c_str(), &addr);
    if(result == 1){
        // success!
    }else if(result == 0){
        SPDLOG_WARN("Invalid ip address:{}", ip);
        IcmpCloseHandle(icmp_handle);
        return false;
    }else{
        SPDLOG_WARN("InetPtonA call failed! last error code:{}", GetLastError());
        IcmpCloseHandle(icmp_handle);
        return false;
    }
    char test_data[32] = "Are you ok?";
    DWORD reply_size = sizeof(ICMP_ECHO_REPLY) + sizeof(test_data);
    LPVOID reply_data_buffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, reply_size);
    if(reply_data_buffer == nullptr){
        SPDLOG_WARN("TestReachableIp: HeapAlloc call failed! last error code:{}", GetLastError());
        IcmpCloseHandle(icmp_handle);
        return false;
    }
    DWORD result_echo_cnt = IcmpSendEcho(  
        icmp_handle,                  // ICMP句柄  
        addr.S_un.S_addr,                     // 目标IP  
        test_data,                   // 发送数据缓存  
        sizeof(test_data),           // 发送数据长度  
        NULL,                       // 可选IP头选项，通常为NULL  
        reply_data_buffer,                // 存储应答结构及数据的缓冲区  
        reply_size,                  // 缓冲区大小  
        100                        // 超时时间(毫秒)  
    );  
    bool reachable = false;
    if(result_echo_cnt > 0){
        auto echo_reply = static_cast<PICMP_ECHO_REPLY>(reply_data_buffer);     
        SPDLOG_INFO("Ip:\"{}\" routing is reachable! Recv echo count is:{}, status code:{} and round trip time:{} ms!", ip, result_echo_cnt, echo_reply->Status, echo_reply->RoundTripTime);
        reachable = (echo_reply->Status == IP_SUCCESS);
    }else{
        SPDLOG_INFO("Ip:{} is not reachable and error code:{}!", ip, GetLastError());
    }
    HeapFree(GetProcessHeap(), 0, reply_data_buffer);  
    IcmpCloseHandle(icmp_handle);  
    return reachable;
}

Tracker::Tracker(const TrackerConfig& config)
    :config_(config)
{
    createTrackerObject();

    timer_ = new QTimer(this);
    timer_->setTimerType(Qt::PreciseTimer);
    timer_->setSingleShot(true);
    connect(timer_, SIGNAL(timeout()), this, SLOT(handleTimeOut()));
}

Tracker::~Tracker()
{
}

void Tracker::createTrackerObject()
{
    if(config_.type == TrackerDeviceType::kNDIVega){
        std::string ip_addr_str = config_.visit_host.toStdString();
        vtkNew<vtkHybridVegaTracker> vega_tracker;
        vega_tracker->SetHostIpPort((char*)ip_addr_str.c_str(), config_.port);        
        vtk_tracker_ = vega_tracker;        
    }else if(config_.type == TrackerDeviceType::kAIM){        
        vtkNew<vtkAIMTracker> aim_tracker;
        aim_tracker->SetReProbeStatusIf(true);
        unsigned char ip[4] = {0};
        QStringList split_strs = config_.visit_host.split('.');
        for(auto i = 0; i < split_strs.size(); ++i){
            ip[i] = split_strs[i].toInt();
        }
        aim_tracker->SetEthernetIp(ip);
        aim_tracker->SetTransmitMedium(TransmitMedium::kEthernet);
        QDir cur_dir(config_.rom_file_dir);
        if(!cur_dir.exists()){
            SPDLOG_WARN("Rom file directory:{} not exists!", config_.rom_file_dir.toStdString());
        }
        QString tool_file_dir = config_.rom_file_dir; 
        aim_tracker->SetToolFileDirectory(tool_file_dir.toLocal8Bit().toStdString().c_str());
        vtk_tracker_ = aim_tracker;
    }else if(config_.type == TrackerDeviceType::kARMD){
        std::string ip_addr_str = config_.visit_host.toStdString();
        vtkNew<vtkARMDTracker> armd_tracker;
        armd_tracker->SetDeviceHostIp(ip_addr_str);
        QDir cur_dir(config_.rom_file_dir);
        if(!cur_dir.exists()){
            SPDLOG_WARN("Rom file directory:{} not exists!", config_.rom_file_dir.toStdString());
        }
        QString tool_file_dir = config_.rom_file_dir;
        armd_tracker->SetConfigToolDirectory(tool_file_dir.toLocal8Bit().toStdString());
        vtk_tracker_ = armd_tracker;
    }    
    if(vtk_tracker_){
        SPDLOG_INFO("Create tracker device successfully, model: \"{}\", visit host:{} port:{}", EnumTraits<TrackerDeviceType>::toString(config_.type), config_.visit_host.toStdString(), config_.port);
        vtk_tracker_->GetWorldCalibrationMatrix()->Identity();
    }
}

vtkTrackerTool* Tracker::getTrackerTool(TrackerToolType t, int id) const
{
    auto config_opt = config_.getToolConfig(t, id);
    if(config_opt){
        return vtk_tracker_->GetTool(config_opt->virtual_port);
    }         
    return nullptr;
}

vtkTrackerTool* Tracker::getTrackerTool(const QString& name) const
{
    auto config_opt = config_.getToolConfig(name);
    if(config_opt){
        return vtk_tracker_->GetTool(config_opt->virtual_port);
    }         
    return nullptr;
}

bool Tracker::checkReachable()
{
    reachable_ = TestReachableIp(config_.visit_host.toStdString());
    return reachable_;
}

bool Tracker::isTracking() const
{
    return vtk_tracker_->IsTracking();
}

bool Tracker::startTracking(bool start_timer_polling)
{
    if(isTracking()){
        return true;
    }
    if(!isReachable()){
        return false;
    }
    loadToolConfigs();
    vtk_tracker_->StartTracking();
    vtk_tracker_->Update();
    if(start_timer_polling){
        startTimerPolling();
    }    
    SPDLOG_INFO("Start tracker successfully!");
    emit trackingStarted();
    return true;
}

void Tracker::stopTracking()
{
    if(!isTracking()){
        return;
    }
    unloadToolConfigs();
    vtk_tracker_->StopTracking();
    stopTimerPolling();
    emit trackingStopped();
}

void Tracker::updateRegistrationMatrix(vtkMatrix4x4* matrix)
{
    vtk_tracker_->SetWorldCalibrationMatrix(matrix);
    emit registrationMatrixUpdated();
}

vtkMatrix4x4* Tracker::getRegistrationMatrix()
{
    return vtk_tracker_->GetWorldCalibrationMatrix();
}

bool Tracker::isRegistrationDone() const
{
    return !vtk_tracker_->GetWorldCalibrationMatrix()->IsIdentity();
}

void Tracker::loadToolConfigs()
{
    QDir cur_dir(config_.rom_file_dir);
    if(!cur_dir.exists()){
        SPDLOG_WARN("Rom file directory:{} not exists!", config_.rom_file_dir.toStdString());
        return;
    }
    tracking_data_.reset();
    tracking_data_.tool_tracking_datas.resize(config_.tool_configs.size());
    for(auto i = 0; i < config_.tool_configs.size(); ++i){
        const auto& cf = config_.tool_configs[i];
        auto& ttd = tracking_data_.tool_tracking_datas[i];
        ttd.reset();
        ttd.id = i;
        ttd.loaded = false;
        if(!cf.isValid()){
            SPDLOG_WARN("Tool:\"{}\" config is invalid!", cf.name.toStdString());
            continue;
        }
        QString rom_filepath = cur_dir.filePath(cf.rom_filename);
        if(!QFileInfo(rom_filepath).exists()){
            SPDLOG_WARN("Tool:\"{}\" rom file:{} not exists!", cf.name.toStdString(), rom_filepath.toStdString());
            continue;
        }
        if(!vtk_tracker_->LoadVirtualSROM(cf.virtual_port, rom_filepath.toLocal8Bit().data())){
            SPDLOG_WARN("Tool:\"{}\" load failed!", cf.name.toStdString());
            continue;
        }
        ttd.loaded = true;
    }
}

void Tracker::unloadToolConfigs()
{
    for(const auto& cf : config_.tool_configs){
        if(IsValidToolPort(cf.virtual_port)){
            vtk_tracker_->ClearVirtualSROM(cf.virtual_port);
        }        
    }
}

//void Tracker::setReferencerEnabled(bool s)
//{
//    auto config_opt = config_.getToolConfig(TrackerToolType::kReferencer, 0);
//    if(config_opt && s){
//        vtk_tracker_->SetReferenceTool(config_opt->virtual_port);
//    }else{
//        vtk_tracker_->SetReferenceTool(kNullToolPort);
//    }
//    SPDLOG_INFO("Reference enabled status:{} changed!", s);
//}

void Tracker::enableReferencer(const QString& name)
{
    auto config_opt = config_.getToolConfig(name);
    if(config_opt && config_opt->type == TrackerToolType::kReferencer){
        vtk_tracker_->SetReferenceTool(config_opt->virtual_port);
        SPDLOG_INFO("Reference enabled!");
    }
}

void Tracker::disableReferencer()
{
    vtk_tracker_->SetReferenceTool(kNullToolPort);
    SPDLOG_INFO("Reference disabled!");
}

bool Tracker::isReferencerEnabled()const
{
    return vtk_tracker_->GetReferenceTool() > 0;
}

TrackerToolStatus Tracker::getToolStatus(TrackerToolType t, int id) const
{
    auto config_opt = config_.getToolConfig(t, id);
    if(config_opt){
        return getToolStatusByPort(config_opt->virtual_port);     
    }
    return TrackerToolStatus::kMissing;
}

TrackerToolStatus Tracker::getToolStatusById(int id) const
{
    auto config_opt = config_.getToolConfig(id);
    if(config_opt){
        return getToolStatusByPort(config_opt->virtual_port);
    }
    return TrackerToolStatus::kMissing;
}

TrackerToolStatus Tracker::getToolStatusByPort(int port) const
{
    TrackerToolStatus tts = TrackerToolStatus::kMissing;
    vtkTrackerTool* tool = vtk_tracker_->GetTool(port);
    if(tool){
        tts = TrackerToolStatus::kNormal;
        if(tool->IsMissing()){
            tts = TrackerToolStatus::kMissing;
        }else if(tool->IsOutOfView()){
            tts = TrackerToolStatus::kOutOfView;
        }else if(tool->IsOutOfVolume()){
            tts = TrackerToolStatus::kOutOfView;
        }
    } 
    return tts;
}

double Tracker::getToolError(TrackerToolType t, int id) const
{
    auto config_opt = config_.getToolConfig(t, id);
    if(config_opt){
        return vtk_tracker_->GetToolError(config_opt->virtual_port);
    }
    return kInvalidToolError;
}

void Tracker::updateTrackingData()
{
    tracking_data_.connected = isTracking();
    if(!tracking_data_.connected){
        return;
    }
    tracking_data_.referencer_enabled = isReferencerEnabled();
    for(auto& td : tracking_data_.tool_tracking_datas){
        auto cf_opt = config_.getToolConfig(td.id);
        if(!cf_opt || !td.loaded){
            continue;
        }        
        const auto& cf = cf_opt.value();
        td.reset();
        vtkTrackerTool* vtk_tool = vtk_tracker_->GetTool(cf.virtual_port);
        td.tool_err = vtk_tool->GetErrors();
        td.status = getToolStatusByPort(cf.virtual_port);
        td.is_valid = (td.status == TrackerToolStatus::kNormal);
        const Point3 kOriginPoint;
        if(cf.tip_point){
            td.tool_pos = vtk_tool->GetTransform()->TransformPoint(*cf.tip_point);
            td.unregistrated_tool_pos = vtk_tool->GetUnCalibrationTransform()->TransformPoint(*cf.tip_point);
        }else{
            td.tool_pos = vtk_tool->GetTransform()->TransformPoint(kOriginPoint);
            td.unregistrated_tool_pos = vtk_tool->GetUnCalibrationTransform()->TransformPoint(kOriginPoint);
        }        
        if(cf.isAxisCalibrated()){            
            const Point3 kAxisPoint(0.0, 0.0, -kDefaultDrillLength);            
            Point3 p = vtk_tool->GetTransform()->TransformPoint(kAxisPoint);
            td.tool_axis_normal = (p - td.tool_pos).normalized();
            p = vtk_tool->GetUnCalibrationTransform()->TransformPoint(kAxisPoint);
            td.unregistrated_tool_axis_normal = (p - td.unregistrated_tool_pos).normalized();
        }
    }
    tracking_data_.marker_number = vtk_tracker_->GetPassiveStrayMarkersNum();
    for(int i = 0; i < tracking_data_.marker_number; i++){
        vtk_tracker_->GetPassiveStrayMarkerPostion(i, tracking_data_.marker_positions[i]);
    }
}

//vtkSmartPointer<vtkTransform> Tracker::getTransformBetweenReferenceAndLocator(bool ref_to_locator)
//{
//	auto ref_to_cam_trans = vtkSmartPointer<vtkTransform>::New();
//	ref_to_cam_trans->Identity();
//    auto hp_tool = getTrackerTool(TrackerToolType::kLocator);
//	ref_to_cam_trans.Get()->DeepCopy(hp_tool->GetUnCalibrationTransform());
//	auto inv_calib_matrix = vtkSmartPointer<vtkMatrix4x4>::New();
//	inv_calib_matrix.Get()->DeepCopy(hp_tool->GetCalibrationMatrix());
//	inv_calib_matrix->Invert();
//	ref_to_cam_trans->Concatenate(inv_calib_matrix);
//	ref_to_cam_trans->Update();
//    if(!ref_to_locator){
//        return ref_to_cam_trans;
//    }
//    ref_to_cam_trans->Inverse();
//    return ref_to_cam_trans;
//}

bool Tracker::startTimerPolling()
{
    if(isTimerPolling()){
        return true;
    }
    if(!isTracking()){
        return false;
    }
    if(timer_->isActive()){
        return true;
    }
    timer_->start(polling_interval_);
    return true;
}

void Tracker::stopTimerPolling()
{
    if(isTimerPolling()){
        timer_->stop();
    }
}

void Tracker::handleTimeOut()
{
    StopWatch sw;
    vtk_tracker_->Update();

    updateTrackingData();

    emit pollingTimeout();

    const int eclapsed_msec = sw.elapsedMillisecs();
    int start_msec = (eclapsed_msec < polling_interval_) ? polling_interval_ - eclapsed_msec : polling_interval_;
    timer_->start(start_msec);
}

bool Tracker::isTimerPolling() const
{
    return timer_->isActive();
}

void Tracker::setTimerPollingInterval(int millsecs)
{
    const int kMinInterval = 15;
    if(millsecs <= kMinInterval){
        SPDLOG_ERROR("New interval:{} value cannot be smaller than the minmum value:{}!", millsecs, kMinInterval);
        return;
    }
    polling_interval_ = millsecs;
}

NAMESPACE_END