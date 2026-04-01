/******************************************************** 
* author: scofieldzhu
* time:2025/11/25
*******************************************************/
#include "trackerToolManagerDlg.h"
#include <QTimer>
#include <QDir>
#include <QMessageBox>
#include "helios/basic/log_service.h"
#include "helios/navigation/drill_config_repository.h"
#include "helios/navigation/device_administrator.h"
#include "helios/core/scene_repository.h"

using namespace helios;

namespace{
    QString GetConfigDirectory()
    {
        static QString app_dir = QDir(QDir().currentPath()).absolutePath();
        return app_dir + "/res/conf";
    }

    void InitDrillLibray()
    {
        QString conf_dir = GetConfigDirectory();
        {
            DrillConfig dcf;
            dcf.name = "short ball seed drill";
            dcf.length = 26.87;
            dcf.diameter = 2.0;            
            QString abs_fn = conf_dir + "/drill/CapSeedDrill.stl";
            dcf.model_filepath = abs_fn;
            DrillConfigRepository::GetInst().addDrillConfig(dcf);
        }
        {
            DrillConfig dcf;
            dcf.name = "TwistDrill";
            dcf.length = 49;
            dcf.diameter = 1.6;
            QString abs_fn = conf_dir + "/drill/TwistDrill.stl";
            dcf.model_filepath = abs_fn;
            DrillConfigRepository::GetInst().addDrillConfig(dcf);
        }              
        {
            DrillConfig dcf;
            dcf.name = "StainlessSteelSandDrill";
            dcf.length = 44;
            dcf.diameter = 1.2;
            QString abs_fn = conf_dir + "/drill/StainlessSteelSandDrill.stl";
            dcf.model_filepath = abs_fn;
            DrillConfigRepository::GetInst().addDrillConfig(dcf);
        }            
    }
}

TrackerToolManagerDlg::TrackerToolManagerDlg(QWidget* p)
	:QWidget(p)
{
	ui_.setupUi(this);
    connect(ui_.show_hp_cb, &QAbstractButton::toggled, this, &TrackerToolManagerDlg::slotShowHpButtonToggled);
    InitDrillLibray();
    for(const auto& key : DrillConfigRepository::GetInst().getAllDrillNames()){
        ui_.drill_cb->addItem(key);
    }
    connect(ui_.drill_cb, &QComboBox::currentTextChanged, this, &TrackerToolManagerDlg::slotCurrentDrillSwitched);
    ui_.drill_cb->setCurrentText("short ball seed drill");
    drill_rotating_timer_ = new QTimer(this);
    connect(drill_rotating_timer_, &QTimer::timeout, this, &TrackerToolManagerDlg::slotDrillRotatingTimeout);
    connect(ui_.rotate_cb, &QAbstractButton::toggled, this, &TrackerToolManagerDlg::slotRotateButtonToggled);
    ui_.drill_cb->setEnabled(false);
    ui_.rotate_cb->setEnabled(false);
    connect(ui_.con_btn, &QAbstractButton::pressed, this, &TrackerToolManagerDlg::slotConBtnPressed);
    connect(ui_.discon_btn, &QAbstractButton::pressed, this, &TrackerToolManagerDlg::slotDisConBtnPressed);
}

TrackerToolManagerDlg::~TrackerToolManagerDlg ()
{

}

void TrackerToolManagerDlg::setRenderWidget(RenderWidget* sw)
{
    render_widget_ = sw;
    if(render_widget_ == nullptr){
        return;
    }
    axial_scene_ = render_widget_->sceneRepository().getScene(Axial);
    coronal_scene_ = render_widget_->sceneRepository().getScene(Coronal);
    sagittal_scene_ = render_widget_->sceneRepository().getScene(Sagittal);
    ortho_scene_ = render_widget_->sceneRepository().getScene(Ortho);
}

void TrackerToolManagerDlg::slotShowHpButtonToggled(bool chk)
{
    if(chk){
        if(handpiece_ == nullptr){
            handpiece_ = std::make_unique<HandpieceSprite>();
            if(!handpiece_->bindTrackerTool(ui_.locator_name_label->text())){
                SPDLOG_ERROR("Bind tracker tool:\"{}\" to handpiece failed!", ui_.locator_name_label->text().toUtf8().toStdString());
                return;
            }
        }
        if(!handpiece_->existsDisplayScene()){
            handpiece_->connectScene(*ortho_scene_);
        }
    }
    handpiece_->setVisibility(chk);
    handpiece_->render();
    ui_.rotate_cb->setEnabled(chk);
    ui_.drill_cb->setEnabled(chk);
}

void TrackerToolManagerDlg::slotCurrentDrillSwitched(const QString &name)
{
    if(handpiece_){
        handpiece_->switchDrill(name);        
        handpiece_->render();
    }
}

void TrackerToolManagerDlg::slotDrillRotatingTimeout()
{
    if(handpiece_){
        handpiece_->getDrill()->rotate();
        handpiece_->render();
    }
}

void TrackerToolManagerDlg::slotRotateButtonToggled(bool chk)
{
    if(chk){
        if(handpiece_ && handpiece_->getVisibility(*ortho_scene_)){
            drill_rotating_timer_->start(0);
        }
    }else{
        if(drill_rotating_timer_->isActive()){
            drill_rotating_timer_->stop();
        }
    }
}

void TrackerToolManagerDlg::slotConBtnPressed()
{
    QString device_config_filepath = GetConfigDirectory() + "/sysbot-device-conf-ndi.json";
    bool init_ok = DeviceAdministrator::GetInst().init(device_config_filepath);
    if(!init_ok){
        SPDLOG_ERROR("Init device administrator failed! config file:\"{}\"", device_config_filepath.toUtf8().toStdString());
        QMessageBox::warning(this, tr("Init device error"), tr("Parse device config file failed!"));
        return;
    }
    auto tracker = DeviceAdministrator::GetInst().getDefaultTracker();
    connect(tracker, &Tracker::trackingStarted, this, &TrackerToolManagerDlg::slotTrackingStarted);
    connect(tracker, &Tracker::trackingStopped, this, &TrackerToolManagerDlg::slotTrackingStopped);
    connect(tracker, &Tracker::pollingTimeout, this, &TrackerToolManagerDlg::slotTrackerTimerPolling);
    if(!tracker->checkReachable()){
        SPDLOG_ERROR("Tracker is not reachable!");
        QMessageBox::warning(this, tr("Route reachable check error"), tr("Tracker is not reachable!"));
        return;
    }
    if(!tracker->startTracking()){
        SPDLOG_ERROR("Start tracker tracking failed!");
        QMessageBox::warning(this, tr("Connect error"), tr("Start tracker failed!"));
        return;
    }
}

void TrackerToolManagerDlg::slotDisConBtnPressed()
{
    auto tracker = DeviceAdministrator::GetInst().getDefaultTracker();
    if(tracker){
        tracker->stopTracking();
    }
}

void TrackerToolManagerDlg::slotTrackingStarted()
{
    Tracker* tracker = static_cast<Tracker*>(sender());
    for(const auto& ttc : tracker->config().tool_configs){
        if(ttc.type == TrackerToolType::kReferencer){
            ui_.referencer_name_label->setText(ttc.name);
        }else if(ttc.type == TrackerToolType::kProbe){
            ui_.probe_name_label->setText(ttc.name);
        }else if(ttc.type == TrackerToolType::kLocator){
            ui_.locator_name_label->setText(ttc.name);
        }
    }
}

void TrackerToolManagerDlg::slotTrackingStopped()
{
    ui_.referencer_name_label->setText("--.--");
    ui_.referencer_pos_label->setText("--.--");
    ui_.probe_name_label->setText("--.--");
    ui_.probe_name_label->setText("--.--");
    ui_.locator_name_label->setText("--.--");
    ui_.locator_name_label->setText("--.--");
}

void TrackerToolManagerDlg::slotTrackerTimerPolling()
{
    Tracker* tracker = static_cast<Tracker*>(sender());
    const auto& td = tracker->currentTrackingData();
    for(const auto& ttd : td.tool_tracking_datas){
        if(ttd.id < 0 || ttd.id >= tracker->config().tool_configs.size()){
            continue;
        }
        const auto& tool_cf = tracker->config().tool_configs[ttd.id];
        if(tool_cf.type == TrackerToolType::kReferencer){            
            ui_.referencer_pos_label->setText(QString::fromStdString(ttd.tool_pos.toStr()));
        }else if(tool_cf.type == TrackerToolType::kProbe){
            ui_.probe_pos_label->setText(QString::fromStdString(ttd.tool_pos.toStr()));
        }else if(tool_cf.type == TrackerToolType::kLocator){            
            ui_.locator_pos_label->setText(QString::fromStdString(ttd.tool_pos.toStr()));
        }
    }
}
