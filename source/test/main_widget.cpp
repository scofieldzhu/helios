#include "main_widget.h"
#include <vtkMetaImageReader.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkImageAccumulate.h>
#include <QTimer>
#include <QResizeEvent>
#include <vtkCallbackCommand.h>
#include <QDoubleValidator>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyle.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include "vtkOrientationMarkerWidget2.h"
#include <vtkAnnotatedCubeActor.h>
#include <vtkMetaImageWriter.h>
#include <vtkSTLReader.h>
#include <vtkProperty.h>
#include <vtkPlane.h>
#include <QColorDialog>
#include <QPluginLoader>
#include <vtkCamera.h>
#include <vtkMatrix3x3.h>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <vtkRenderer.h>
#include "helios/sprites/point_cloud_sprite.h"
#include "helios/sprites/composite_volume_sprite.h"
#include "helios/sprites/surface_volume_sprite.h"
#include "helios/core/slice_plane_sprite.h"
#include "helios/sprites/ortho_planes_sprite.h"
#include "helios/basic/image_data_util.h"
#include "helios/sprites/plane_intersection_line_sprite.h"
#include "helios/sprites/polygon_sprite.h"
#include "helios/basic/log_service.h"
#include "helios/core/sprite_cell_picker.h"
#include "helios/sprites/plane_intersection_line_editor.h"
#include "helios/sprites/sphere_sprite.h"
#include "helios/sprites/line_sprite.h"
#include "helios/sprites/polygon_editor.h"
#include "helios/sprites/label_sprite.h"
#include "helios/sprites/polydata_cutter_sprite.h"
#include "helios/core/axial_plane_scene.h"
#include "helios/core/coronal_plane_scene.h"
#include "helios/core/sagittal_plane_scene.h"
#include "raycasting_config_load.h"
#include "IViewer.h"
#include "IPathDesigner.h"
#include "ISceneWidgetSource.h"
#include "ITrackerToolManager.h"
#include "IDICOMReader.h"

using namespace helios;

namespace {
    vtkSmartPointer<vtkLookupTable> CreateDefautlLookupTable(vtkImageData* data)
    {
        vtkNew<vtkImageAccumulate> imageAccumulate;
        imageAccumulate->SetInputData(data);
        imageAccumulate->Update();
        double minVal = imageAccumulate->GetMin()[0];
        double maxVal = imageAccumulate->GetMax()[0];
        vtkNew<vtkLookupTable> lookup_table;
        lookup_table->SetNumberOfTableValues(256);
        lookup_table->Build();
        for (int idx = 0; idx < 256; ++idx)
            lookup_table->SetTableValue(idx, idx / 255.0, idx / 255.0, idx / 255.0);
        lookup_table->SetTableRange(minVal, maxVal);
        return lookup_table;
    }

    constexpr double kBorder = 0.01;
    constexpr double kNormalSceneWidth  = (1.0 - 3.0 * kBorder) / 2.0;
    constexpr double kNormalSceneHeight = (1.0 - 3.0 * kBorder) / 2.0;
    constexpr double kSceneToolbarHeight = 36;   

    QString GetPluginDirectory()
    {
    #ifdef BUILD_TYPE_DEBUG
        QString plugin_dir = QDir(QDir().currentPath()).absolutePath() + "/plugins/Debug";
    #elif BUILD_TYPE_RELWITHDEBINFO
        QString plugin_dir = QDir(QDir().currentPath()).absolutePath() + "/plugins/RelWithDebInfo";
    #else
        QString plugin_dir = QDir(QDir().currentPath()).absolutePath() + "/plugins/Release";
    #endif
        if(!QFileInfo(plugin_dir).exists()){
            plugin_dir = QDir(QDir().currentPath()).absolutePath() + "/plugins";
        }
        return plugin_dir;
    }

    QString GetConfigDirectory()
    {
        static QString app_dir = QDir(QDir().currentPath()).absolutePath();
        return app_dir + "/res/conf";
    }    
}

MainWidget::MainWidget(QWidget *parent)
    :QMainWindow(parent),
    volume_data_(vtkSmartPointer<vtkImageData>::New())
{
    ui_.setupUi(this);
    connect(ui_.load_dicom_btn, &QAbstractButton::pressed, this, &MainWidget::slotLoadDicomBtnPressed);
    //connect(ui_.show_point_cloud_cb, &QAbstractButton::toggled, this, &MainWidget::slotShowPointCloudBtnClicked);
    connect(ui_.show_volume_cb, &QAbstractButton::toggled, this, &MainWidget::slotShowVolumeBtnClicked);
    connect(ui_.show_orthoplanes_cb, &QAbstractButton::toggled, this, &MainWidget::slotShowOrthoPlanesBtnClicked);
    connect(ui_.mpr_cb, &QAbstractButton::toggled, this, &MainWidget::slotMprCbClicked);
    //QDoubleValidator* dv = new QDoubleValidator(0.0, 20.0, 1, ui_.point_size_edit);
    //ui_.point_size_edit->setValidator(dv);
    //connect(ui_.point_size_edit, &QLineEdit::returnPressed, this, &MainWidget::slotPointSizeReturnPressed);
    connect(ui_.draw_polygon_btn, &QAbstractButton::toggled, this, &MainWidget::slotDrawPolygonButtonClicked);    
    connect(ui_.del_polygon_btn, &QAbstractButton::pressed, this, &MainWidget::slotDeletePolygonButtonPressed);
    ui_.render_mode_cb->addItem(tr("Composite"), VolumeRenderMode::VRM_COMPOSITION);
    ui_.render_mode_cb->addItem(tr("Surface"), VolumeRenderMode::VRM_SURFACE);
    ui_.render_mode_cb->setCurrentIndex(-1);
    connect(ui_.render_mode_cb, &QComboBox::currentTextChanged, this, &MainWidget::slotCurrentRenderModeChanged);       
    sys_raycasting_conf_gp_ = LoadConfigFile(GetConfigDirectory() + "/lookuptable-system-conf.json");
    for(const auto& config : sys_raycasting_conf_gp_){
        ui_.lookup_table_cb->addItem(config.name);
    }
    ui_.lookup_table_cb->setCurrentIndex(-1);
    connect(ui_.lookup_table_cb, &QComboBox::currentTextChanged, this, &MainWidget::slotCurrentLookupTableChanged);
    connect(ui_.interpolate_cb, &QCheckBox::toggled, this, &MainWidget::slotInterpolateButtonToggled);
    connect(ui_.orientation_marker_cb, &QCheckBox::toggled, this, &MainWidget::slotShowOrientationMarker);
    connect(ui_.adjust_wlw_cb, &QCheckBox::toggled, this, &MainWidget::slotAdjustWLW);
    connect(ui_.vol_opacity_slider, &QAbstractSlider::valueChanged, this, &MainWidget::slotOpacitySliderValueChanged);
    connect(this, &MainWidget::globalImageUpdated, this, &MainWidget::slotGlobalImageUpdated);
    scanPluginDirectory();
}

MainWidget::~MainWidget()
{    
    pm_.unloadAll(); // break relationship with SceneWidget at first avoid dependency error!
    if(axial_scene_){
        delete axial_scene_;
        axial_scene_ = nullptr;
    }
    if(coronal_scene_){
        delete coronal_scene_;
        coronal_scene_ = nullptr;
    }
    if(sagittal_scene_){
        delete sagittal_scene_;
        sagittal_scene_ = nullptr;
    }
    if(ortho_scene_){
        delete ortho_scene_;
        ortho_scene_ = nullptr;
    }
}

VolumeRenderMode MainWidget::getVolumeRenderMode() const
{
    return static_cast<VolumeRenderMode>(ui_.render_mode_cb->itemData(ui_.render_mode_cb->currentIndex()).toInt());
}

void MainWidget::setGlobalImage(vtkSmartPointer<vtkImageData> d)
{
    volume_data_->DeepCopy(d);
    SPDLOG_INFO("Global image copyed yet!");
}

void MainWidget::slotShowPointCloudBtnClicked(bool chk)
{
    auto pc = ortho_scene_->getSprite("PointCloud");
    if(chk){
        if(pc == nullptr){
            auto new_pc = new PointCloudSprite(PointCloudSprite::PT_VERTEX);
            new_pc->setName("PointCloud");
            new_pc->setColor(Color::Purple);
            new_pc->setPoints({{0.0, 0.0, 0.0}, {0.0, 0.0, 10.0}, {0.0, 10.0, 0.0}});
            new_pc->connectScene(*ortho_scene_);
            ortho_scene_->resetView();
            pc = new_pc;
        }
        pc->setVisibility(1);
        pc->render();        
    }else{
        if(pc){
            pc->setVisibility(0);
            pc->render();        
        }
    }
}

void MainWidget::slotShowVolumeBtnClicked(bool chk)
{
    SPDLOG_DEBUG("slotShowVolumeBtnClicked called!");
    if(!chk){
        ui_.label_3->setEnabled(false);
        ui_.render_mode_cb->setEnabled(false);       
    }else{
        ui_.render_mode_cb->setEnabled(true);
        ui_.label_3->setEnabled(true);
        auto mode = getVolumeRenderMode();        
    }
    ui_.label_4->setEnabled(chk);
    ui_.lookup_table_cb->setEnabled(chk && getVolumeRenderMode() == VRM_COMPOSITION);
    auto vs = ortho_scene_->getSprite("Volume");
    vs->setVisibility(chk);
    vs->render();  
}

void MainWidget::slotCurrentRenderModeChanged(const QString &name)
{
    auto mode = getVolumeRenderMode();        
    ui_.lookup_table_cb->setEnabled(mode != VRM_SURFACE);
    if(volume_data_){
        createVolumeSprite(volume_data_);
        ortho_scene_->render();
    } 
    if(mode != VRM_SURFACE){
        forceApplyCurrentRaycatingConfig(); 
    }  
}

void MainWidget::slotShowOrthoPlanesBtnClicked(bool chk)
{
    SPDLOG_DEBUG("slotShowOrthoPlanesBtnClicked called!");
    if(orthoplanes_){
        orthoplanes_->setSceneVisibility(*ortho_scene_, chk);
        orthoplanes_->render();  
    }
}

void MainWidget::slotCurrentLookupTableChanged(const QString& text)
{
    auto existing_volume = AbstractVolumeSprite::SafeDownCast(ortho_scene_->getSprite("Volume"));
    if(existing_volume && getVolumeRenderMode() != VRM_SURFACE){            
        applyRaycastingConfig(text, existing_volume->getVolumeProperty());
        existing_volume->modified();
        existing_volume->render();
    }
}

void MainWidget::slotMprCbClicked(bool chk)
{
    SPDLOG_TRACE("MPR button checked:{}", chk);
    // if(chk){
    //     pil_editor_->setCurrentScene(axial_scene_);
    // }
    axial_pil_editor_->setEnabled(chk);
    coronal_pil_editor_->setEnabled(chk);
    sagittal_pil_editor_->setEnabled(chk);
}

//void MainWidget::slotPointSizeReturnPressed()
//{
//    auto pc = ortho_scene_->getSprite("PointCloud");
//    if(pc){
//        auto pcs = dynamic_cast<PointCloudSprite*>(pc);
//        double new_size = ui_.point_size_edit->text().toDouble();
//        pcs->setPointSize(new_size);
//        pc->render();     
//    }
//}

void MainWidget::slotMaximizeButtonClicked(bool chk)
{
    auto event_button = qobject_cast<QPushButton*>(sender());
    auto event_scene_name = event_button->property("Scene").toString();
    if(chk){
        event_button->setText("Restore");
        auto s = maximizeSceneViewport(event_scene_name.toStdString());
        maximizeToolbarPos(event_button, s);        
    }else{
        event_button->setText("Maximize");
        restoreSceneViewport();
        restoreToolbarPos();
    }
    resetPlaneView();
}

void MainWidget::slotAxialSlicePositionChanged(int value)
{
    double space_z = volume_data_->GetSpacing()[2];
	double pos = value * space_z;
    if(orthoplanes_){
	    orthoplanes_->getAxialPlane()->setPosition(pos);
        ui_.render_widget->render();
    }
}

void MainWidget::slotCoronalSlicePositionChanged(int value)
{
    double space_y = volume_data_->GetSpacing()[1];
	double pos = value * space_y;
    if(orthoplanes_){
        orthoplanes_->getCoronalPlane()->setPosition(pos);
        ui_.render_widget->render();
    }	
}

void MainWidget::slotSagittalSlicePositionChanged(int value)
{
    double space_x = volume_data_->GetSpacing()[0];
	double pos = value * space_x;
    if(orthoplanes_){
        orthoplanes_->getSagittalPlane()->setPosition(pos);
        ui_.render_widget->render();
    }	
}

void MainWidget::slotDrawPolygonButtonClicked(bool chk)
{
    if(chk){
        if(polygon_ == nullptr){
            polygon_ = std::make_unique<PolygonSprite>();
            polygon_->setLineWidth(2.5);
            polygon_->connectScenes({ortho_scene_});
        }
        polygon_editor_ = vtkSmartPointer<PolygonEditor>::New();
        polygon_editor_->setCurrentInteractor(ui_.render_widget->interactor());
        polygon_editor_->setCurrentSprite(polygon_.get());
        auto vs = AbstractVolumeSprite::SafeDownCast(ortho_scene_->getSprite("Volume"));
        polygon_editor_->setCurrentVolume(vs->getVolume());
        polygon_editor_->setAllowedScenes(polygon_->getDisplaySceneList());
        polygon_editor_->setEnabled(true);
    }else{
        if(polygon_editor_){
            polygon_editor_->setEnabled(false);
        }
        polygon_editor_ = nullptr;
    }
    ui_.render_widget->render();
}

void MainWidget::slotDeletePolygonButtonPressed()
{
    polygon_ = nullptr;
    ui_.render_widget->render();
}

void MainWidget::slotInterpolateButtonToggled(bool chk)
{
    if(orthoplanes_){
        orthoplanes_->setTextureInterpolate(chk);
        orthoplanes_->render();
    }
}
void MainWidget::slotShowOrientationMarker(bool chk)
{
    if(chk){        
		for(auto i = 0; i < 4; ++i){
            orientation_marker_widgets_[i]->GetOrientationMarker()->SetVisibility(1);	
            orientation_marker_widgets_[i]->GetOrientationMarker()->Modified();
        }
    }else{
        for(auto i = 0; i < 4; ++i){
            orientation_marker_widgets_[i]->GetOrientationMarker()->SetVisibility(0);	
            orientation_marker_widgets_[i]->GetOrientationMarker()->Modified();
        }
    }
    ui_.render_widget->render();
}

void MainWidget::OnWLWUpdating(vtkObject* caller, unsigned long eid, void* client_data, void* call_data)
{
    auto mw = static_cast<MainWidget*>(client_data);
    WLWEditor::CallData* data = reinterpret_cast<WLWEditor::CallData*>(call_data);
    mw->onWLWUpdated(data->event_scene, data->wl, data->ww);
}

void MainWidget::slotAdjustWLW(bool chk)
{
    if(change_threshold_range_editor_ == nullptr){
        change_threshold_range_editor_ = vtkSmartPointer<WLWEditor>::New();
        change_threshold_range_editor_->setAllowedScenes({ortho_scene_, axial_scene_, coronal_scene_, sagittal_scene_});
        change_threshold_range_editor_->setCurrentInteractor(ui_.render_widget->interactor());        
        vtkNew<vtkCallbackCommand> cb;
        cb->SetCallback(MainWidget::OnWLWUpdating);
        cb->SetClientData(this);
        change_threshold_range_editor_->AddObserver(WLWEditor::WLW_UPDATE_EVENT, cb);    
    }
    if(chk){        
        auto existing_volume = SurfaceVolumeSprite::SafeDownCast(ortho_scene_->getSprite("Volume"));  
        change_threshold_range_editor_->setVolume(existing_volume);
        change_threshold_range_editor_->setOrthoPlanes(orthoplanes_);
        change_threshold_range_editor_->setEnabled(true);
    }else{
        change_threshold_range_editor_->setEnabled(false);
        ui_.render_widget->render();
    }
}

void MainWidget::slotOpacitySliderValueChanged(int value)
{
    double op = ((double)value) / (ui_.vol_opacity_slider->maximum() - ui_.vol_opacity_slider->minimum());
    auto existing_volume = AbstractVolumeSprite::SafeDownCast(ortho_scene_->getSprite("Volume"));  
    if(existing_volume){
        existing_volume->setOpacity(std::clamp(op, 0.0, 1.0));
        existing_volume->render();
    }
}

void MainWidget::onImageReadFinished(IDICOMReader* reader)
{
    setGlobalImage(reader->getSeriesData());
    emit globalImageUpdated();
}

void MainWidget::slotLoadDicomBtnPressed()
{
    auto target_indexes = pm_.indexByIID(IDICOM_READER_IID);
    if(!target_indexes.empty()){
        pm_.unload(target_indexes[0]);
    }else{
        QMessageBox::warning(this, tr("Error tip dialog"), tr("Cannot get Read plugin object!"), QMessageBox::Ok);
        return;
    } 

    QString user_dicom_dir = QFileDialog::getExistingDirectory(
        this,                         
        tr("Choose dicom directory"),               
        "./",                        
        QFileDialog::ShowDirsOnly
    );
    if(user_dicom_dir.isEmpty()) {
        return;
    }
    SPDLOG_INFO("Chosen dicom directory:{}", user_dicom_dir.toUtf8().toStdString());

    auto objs = pm_.loadAllByIID(IDICOM_READER_IID);
    if(objs.empty() || qobject_cast<IDICOMReader*>(objs[0]) == nullptr){
        SPDLOG_WARN("Load plugin:\"{}\" failed!", IDICOM_READER_IID);
        QMessageBox::warning(this, tr("Error tip dialog"), tr("Load DICOM reader plugin error!"), QMessageBox::Ok);
        return;
    }
    auto reader = qobject_cast<IDICOMReader*>(objs[0]);
    reader->setDICOMDirectory(user_dicom_dir);
    using namespace std::placeholders;
    reader->setFinishReadCallback(std::bind(&MainWidget::onImageReadFinished, this, _1));
    auto plugin_viewer = qobject_cast<IViewer*>(objs[0]);
    if(plugin_viewer == nullptr){
        return;
    }
    plugin_viewer->createView(this);
}

void MainWidget::slotGlobalImageUpdated()
{
    initUI();
    auto a_n = orthoplanes_->getAxialPlane()->getNormal();
    auto c_n = orthoplanes_->getCoronalPlane()->getNormal();
    auto s_n = orthoplanes_->getSagittalPlane()->getNormal();
    SPDLOG_DEBUG("a_n:{} c_n:{} s_n:{}", a_n.toStr(), c_n.toStr(), s_n.toStr());
}

vtkSmartPointer<vtkCallbackCommand> stRotatingCallback;
// SphereSprite* stSphere = nullptr;
// Scene* stOrthoScene = nullptr;

void OnRotating(vtkObject* o, unsigned long, void*, void*)
{
    // if(stSphere == nullptr){
    //     stSphere = new SphereSprite();
    //     stSphere->setColor(Color::Red);
    //     stSphere->setRadius(2.0);
    //     stSphere->connectScene(*stOrthoScene);
    // }
    // auto center = stOrthoPlanes->calcOrthoCenter();
    // stSphere->setCenter(center);
    // stSphere->render();    
}

void MainWidget::createVolumeSprite(vtkImageData* vol_data)
{
    auto existing_volume = ortho_scene_->getSprite("Volume");
    if(existing_volume){
        existing_volume->disconnectScene(*ortho_scene_);
        delete existing_volume;
    }
    double range[2];
    vol_data->GetScalarRange(range);
    auto mode = getVolumeRenderMode();
    AbstractVolumeSprite* new_vs = nullptr;
    if(mode == VRM_COMPOSITION){
        new_vs = new CompositeVolumeSprite(1024 * 1024 * 4096);
    }else{
        auto svs = new SurfaceVolumeSprite(1024 * 1024 * 4096);
        svs->setVolumeThresholdValue(5, 300, 2000);
        new_vs = svs;
    }
    new_vs->setName("Volume");            
    new_vs->connectScene(*ortho_scene_);
    new_vs->setInputData(vol_data);
    ortho_scene_->getRenderer()->ResetCamera();        
    ortho_scene_->getRenderer()->GetActiveCamera()->SetViewUp(0.0, 1.0, 0.0);
}

void MainWidget::forceApplyCurrentRaycatingConfig()
{
    auto existing_volume = AbstractVolumeSprite::SafeDownCast(ortho_scene_->getSprite("Volume"));
    if(existing_volume == nullptr){   
        return;
    }
    auto lookup_item_name = ui_.lookup_table_cb->currentText();
    applyRaycastingConfig(lookup_item_name, existing_volume->getVolumeProperty());
    existing_volume->modified();
    existing_volume->render();
}

void MainWidget::applyRaycastingConfig(const QString &name, vtkVolumeProperty *vol_prop)
{
    auto it = std::find_if(sys_raycasting_conf_gp_.begin(), sys_raycasting_conf_gp_.end(), [&name](const RaycastingConf& conf){
        return conf.name == name;
    });
    if(it == sys_raycasting_conf_gp_.end()){
        return;
    }
    auto right_config = *it;
    vol_prop->SetShade(right_config.lightness.has_value() ? 1 : 0);
    if(right_config.lightness){
        auto l = right_config.lightness.value();
        vol_prop->SetAmbient(l.ambient);
        vol_prop->SetSpecular(l.specular);
        vol_prop->SetDiffuse(l.diffuse);
        vol_prop->SetSpecularPower(3.0);
    }
    auto clr_func = vol_prop->GetRGBTransferFunction(0);
    auto opacity_func = vol_prop->GetScalarOpacity(0);
    clr_func->RemoveAllPoints();
    clr_func->SetClamping(0);
    opacity_func->RemoveAllPoints();
    opacity_func->SetClamping(0);
    for(const auto& ctrlpt_conf : right_config.ctrl_points){
        clr_func->AddRGBPoint(ctrlpt_conf.intensity, ctrlpt_conf.clr.redR(), ctrlpt_conf.clr.greenR(), ctrlpt_conf.clr.blueR());
        opacity_func->AddPoint(ctrlpt_conf.intensity, ctrlpt_conf.opacity);
    }
    clr_func->SetClamping(1);
    opacity_func->SetClamping(1);
    clr_func->Modified();
    opacity_func->Modified();
    vol_prop->Modified();

    if(orthoplanes_ == nullptr || !right_config.window_level.has_value()){
        return;
    }
    auto lookup_table = orthoplanes_->getLookupTable(0);
    double low = (*right_config.window_level)[0] - (*right_config.window_level)[1] / 2.0;
    double high = (*right_config.window_level)[0] + (*right_config.window_level)[1] / 2.0;
    lookup_table->SetTableRange(low, high);
    lookup_table->Modified();
    orthoplanes_->modified();
}

void MainWidget::setThresholdRange(int lower, int high)
{
    if(getVolumeRenderMode() == VRM_SURFACE){
        auto vs = SurfaceVolumeSprite::SafeDownCast(ortho_scene_->getSprite("Volume"));
        if(vs){
            vs->setVolumeThresholdValue(100, lower, high);
            vs->render();
        }
    }    
}

void MainWidget::initModels(vtkImageData *vol_data)
{
    auto default_lookuptable = CreateDefautlLookupTable(vol_data);

    if(orthoplanes_ == nullptr){
        orthoplanes_ = new OrthoPlanesSprite();
        orthoplanes_->setName("OrthoPlanes");
        orthoplanes_->addInputData(vol_data);        
    }else{
        orthoplanes_->setInputData(vol_data);
    }        
    orthoplanes_->setLookupTable(default_lookuptable, 0);
    orthoplanes_->connectScene(*ortho_scene_);
    orthoplanes_->setSceneVisibility(*ortho_scene_, 0);
    
    using namespace std::placeholders;

    auto axial_plane = orthoplanes_->getAxialPlane();
    PlaneScene::SafeDownCast(axial_scene_)->setPlane(axial_plane);
    axial_plane->PositionChangeSignal.clear();
    axial_plane->PositionChangeSignal.bind(std::bind(&MainWidget::onAxialSlicePositionChanged, this, _1, _2));
    axial_plane->PushRangeChangeSignal.clear();
    axial_plane->PushRangeChangeSignal.bind(std::bind(&MainWidget::onAxialSliceRangeChanged, this, _1, _2));
    auto data_tuple = CalcPushRangeAndStartPointOfImagePlane(vol_data, axial_plane->getNormal());
    axial_plane->setStartPoint(std::get<Point3>(data_tuple));
    axial_plane->setPushRange(std::get<double>(data_tuple));
    axial_plane->connectScene(*axial_scene_);    

    auto coronal_plane = orthoplanes_->getCoronalPlane();
    PlaneScene::SafeDownCast(coronal_scene_)->setPlane(coronal_plane);
    coronal_plane->PositionChangeSignal.clear();
    coronal_plane->PositionChangeSignal.bind(std::bind(&MainWidget::onCoronalSlicePositionChanged, this, _1, _2));
    coronal_plane->PushRangeChangeSignal.clear();
    coronal_plane->PushRangeChangeSignal.bind(std::bind(&MainWidget::onCoronalSliceRangeChanged, this, _1, _2));    
    data_tuple = CalcPushRangeAndStartPointOfImagePlane(vol_data, coronal_plane->getNormal());
    coronal_plane->setStartPoint(std::get<Point3>(data_tuple));
    coronal_plane->setPushRange(std::get<double>(data_tuple));
    coronal_plane->connectScene(*coronal_scene_);

    auto sagittal_plane = orthoplanes_->getSagittalPlane();   
    PlaneScene::SafeDownCast(sagittal_scene_)->setPlane(sagittal_plane);
    sagittal_plane->PositionChangeSignal.clear();
    sagittal_plane->PositionChangeSignal.bind(std::bind(&MainWidget::onSagittalSlicePositionChanged, this, _1, _2));
    sagittal_plane->PushRangeChangeSignal.clear();
    sagittal_plane->PushRangeChangeSignal.bind(std::bind(&MainWidget::onSagittalSliceRangeChanged, this, _1, _2));
    data_tuple = CalcPushRangeAndStartPointOfImagePlane(vol_data, sagittal_plane->getNormal());
    sagittal_plane->setStartPoint(std::get<Point3>(data_tuple));
    sagittal_plane->setPushRange(std::get<double>(data_tuple));
    sagittal_plane->connectScene(*sagittal_scene_);

    crosslines_.clear();

    auto crossline = std::make_unique<PlaneIntersectionLineSprite>();
    crossline->setName("AxialPlaneIntersectionLine");
    crossline->setLineColor(Color::Green, Color::Blue);
    crossline->setPlanes(*coronal_plane, *axial_plane, *sagittal_plane);
    crossline->connectScene(*axial_scene_);
    PlaneIntersectionLineSprite* axial_intersection_line = crossline.get();
    crosslines_.emplace_back(std::move(crossline));

    axial_pil_editor_ = PlaneIntersectionLineEditor::New();
    axial_pil_editor_->setEventPriority(0.1);
    axial_pil_editor_->setCurrentInteractor(ui_.render_widget->interactor());
    axial_pil_editor_->setCurrentSprite(axial_intersection_line);
    axial_pil_editor_->setAllowedScenes({axial_scene_});

    //stRotatingCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    //stRotatingCallback->SetCallback(OnRotating);
    //axial_pil_editor_->AddObserver(PlaneIntersectionLineEditor::ROTATING_EVENT, stRotatingCallback);
    //axial_pil_editor_->AddObserver(PlaneIntersectionLineEditor::TRANSLATING_EVENT, stRotatingCallback);

    crossline = std::make_unique<PlaneIntersectionLineSprite>();
    crossline->setName("CoronalPlaneIntersectionLine");
    crossline->setLineColor(Color::Red, Color::Blue);
    crossline->setPlanes(*axial_plane, *coronal_plane, *sagittal_plane);
    crossline->connectScene(*coronal_scene_);
    PlaneIntersectionLineSprite* coronal_intersection_line = crossline.get();
    crosslines_.emplace_back(std::move(crossline));

    coronal_pil_editor_ = PlaneIntersectionLineEditor::New();
    coronal_pil_editor_->setEventPriority(0.2);
    coronal_pil_editor_->setCurrentInteractor(ui_.render_widget->interactor());
    coronal_pil_editor_->setCurrentSprite(coronal_intersection_line);
    coronal_pil_editor_->setAllowedScenes({coronal_scene_});
    //coronal_pil_editor_->AddObserver(PlaneIntersectionLineEditor::ROTATING_EVENT, stRotatingCallback);
    //coronal_pil_editor_->AddObserver(PlaneIntersectionLineEditor::TRANSLATING_EVENT, stRotatingCallback);

    crossline = std::make_unique<PlaneIntersectionLineSprite>();
    crossline->setName("SagittalPlaneIntersectionLine");
    crossline->setLineColor(Color::Red, Color::Green);
    crossline->setPlanes(*axial_plane, *sagittal_plane, *coronal_plane);
    crossline->connectScene(*sagittal_scene_);
    PlaneIntersectionLineSprite* sagittal_intersection_line = crossline.get();
    crosslines_.emplace_back(std::move(crossline));

    sagittal_pil_editor_ = PlaneIntersectionLineEditor::New();
    sagittal_pil_editor_->setEventPriority(0.3);
    sagittal_pil_editor_->setCurrentInteractor(ui_.render_widget->interactor());
    sagittal_pil_editor_->setCurrentSprite(sagittal_intersection_line);
    sagittal_pil_editor_->setAllowedScenes({sagittal_scene_});

    //sagittal_pil_editor_->AddObserver(PlaneIntersectionLineEditor::ROTATING_EVENT, stRotatingCallback);
    //sagittal_pil_editor_->AddObserver(PlaneIntersectionLineEditor::TRANSLATING_EVENT, stRotatingCallback);

    createVolumeSprite(vol_data);    
}

void MainWidget::onAxialSlicePositionChanged(helios::SlicePlaneSprite* plane, double position)
{
    axial_toolbar_->slice_slider->blockSignals(true);
    double spacing = volume_data_->GetSpacing()[2];
    int new_slider_pos = (int)(position / spacing);
    int old_slider_pos = axial_toolbar_->slice_slider->value();
    if(new_slider_pos != old_slider_pos){
        axial_toolbar_->slice_slider->setValue(new_slider_pos);
    }
    axial_toolbar_->slice_slider->blockSignals(false);
}

void MainWidget::onAxialSliceRangeChanged(helios::SlicePlaneSprite* plane, double range)
{
    axial_toolbar_->slice_slider->blockSignals(true);
    double spacing = volume_data_->GetSpacing()[2];
    int new_slider_range = range / spacing;
    int old_slider_range = axial_toolbar_->slice_slider->maximum() - axial_toolbar_->slice_slider->minimum() + 1;
    if(new_slider_range != old_slider_range){
        axial_toolbar_->slice_slider->setRange(0, new_slider_range);
    }
    axial_toolbar_->slice_slider->blockSignals(false);
}

void MainWidget::onCoronalSlicePositionChanged(helios::SlicePlaneSprite *plane, double position)
{
    coronal_toolbar_->slice_slider->blockSignals(true);
    double spacing = volume_data_->GetSpacing()[1];
    int new_slider_pos = (int)(position / spacing);
    int old_slider_pos = coronal_toolbar_->slice_slider->value();
    if(new_slider_pos != old_slider_pos){
        coronal_toolbar_->slice_slider->setValue(new_slider_pos);
    }
    coronal_toolbar_->slice_slider->blockSignals(false);
}

void MainWidget::onCoronalSliceRangeChanged(helios::SlicePlaneSprite *plane, double range)
{
    coronal_toolbar_->slice_slider->blockSignals(true);
    double spacing = volume_data_->GetSpacing()[1];
    int new_slider_range = range / spacing;
    int old_slider_range = coronal_toolbar_->slice_slider->maximum() - coronal_toolbar_->slice_slider->minimum() + 1;
    if(new_slider_range != old_slider_range){
        coronal_toolbar_->slice_slider->setRange(0, new_slider_range);
    }
    coronal_toolbar_->slice_slider->blockSignals(false);
}

void MainWidget::onSagittalSlicePositionChanged(helios::SlicePlaneSprite *plane, double position)
{
    sagittal_toolbar_->slice_slider->blockSignals(true);
    double spacing = volume_data_->GetSpacing()[0];
    int new_slider_pos = (int)(position / spacing);
    int old_slider_pos = sagittal_toolbar_->slice_slider->value();
    if(new_slider_pos != old_slider_pos){
        sagittal_toolbar_->slice_slider->setValue(new_slider_pos);
    }
    sagittal_toolbar_->slice_slider->blockSignals(false);
}

void MainWidget::onSagittalSliceRangeChanged(helios::SlicePlaneSprite* plane, double range)
{
    sagittal_toolbar_->slice_slider->blockSignals(true);
    double spacing = volume_data_->GetSpacing()[0];
    int new_slider_range = range / spacing;
    int old_slider_range = sagittal_toolbar_->slice_slider->maximum() - sagittal_toolbar_->slice_slider->minimum() + 1;
    if(new_slider_range != old_slider_range){
        sagittal_toolbar_->slice_slider->setRange(0, new_slider_range);
    }
    sagittal_toolbar_->slice_slider->blockSignals(false);
}

void MainWidget::createOrientationMarkerWidgets()
{
    auto new_cube_actor_func = [this](Scene* s){
        vtkNew<vtkAnnotatedCubeActor> cube_actor;
        cube_actor->SetXPlusFaceText("L");
        cube_actor->SetXMinusFaceText("R");
        cube_actor->SetYMinusFaceText("A");
        cube_actor->SetYPlusFaceText("P");
        cube_actor->SetZMinusFaceText("I");
        cube_actor->SetZPlusFaceText("S");
        cube_actor->GetTextEdgesProperty()->SetColor(Color::Yellow.toValuesR().data());
        cube_actor->GetTextEdgesProperty()->SetLineWidth(2);
        cube_actor->GetCubeProperty()->SetColor(Color::Blue.toValuesR().data());
        auto new_marker_widget = vtkSmartPointer<vtkOrientationMarkerWidget2>::New();
        new_marker_widget->SetOutlineColor(0.9300, 0.5700, 0.1300);	
        new_marker_widget->SetOrientationMarker(cube_actor);
        new_marker_widget->SetDefaultRenderer(s->getRenderer());
        new_marker_widget->SetViewport(0.01, 0.01, 0.21, 0.21);
        new_marker_widget->SetInteractor(this->ui_.render_widget->interactor());		        
        new_marker_widget->EnabledOn();        		
        new_marker_widget->InteractiveOff();
        return new_marker_widget;
    };
    orientation_marker_widgets_[0] = nullptr;
    orientation_marker_widgets_[0] = new_cube_actor_func(coronal_scene_);
    orientation_marker_widgets_[0]->Modified();
    orientation_marker_widgets_[1] = nullptr;
    orientation_marker_widgets_[1] = new_cube_actor_func(axial_scene_);
    orientation_marker_widgets_[1]->Modified();
    orientation_marker_widgets_[2] = nullptr;
    orientation_marker_widgets_[2] = new_cube_actor_func(sagittal_scene_);
    orientation_marker_widgets_[2]->Modified();
    orientation_marker_widgets_[3] = nullptr;
    orientation_marker_widgets_[3] = new_cube_actor_func(ortho_scene_);
    orientation_marker_widgets_[3]->Modified();
    ui_.render_widget->render();
}

void MainWidget::onWLWUpdated(Scene* scene, double wl, double ww)
{    
    std::string wlw_text = std::format("WL:{} WW:{}", wl, ww);
    if(PlaneScene::SafeDownCast(scene)){ // scene->isPlaneMode()
        scene_wlw_labels_[axial_scene_]->setText(wlw_text.c_str());
        scene_wlw_labels_[coronal_scene_]->setText(wlw_text.c_str());
        scene_wlw_labels_[sagittal_scene_]->setText(wlw_text.c_str());
    }else{
        scene_wlw_labels_[scene]->setText(wlw_text.c_str());
    }
    scene_wlw_labels_[scene]->render();
}

void MainWidget::handleInteractorEvent(vtkObject* caller, unsigned long e_id)
{
    auto iren = vtkRenderWindowInteractor::SafeDownCast(caller);    
    int x = iren->GetEventPosition()[0];
    int y = iren->GetEventPosition()[1];    
    if(e_id == vtkCommand::MouseWheelForwardEvent || e_id == vtkCommand::MouseWheelBackwardEvent){
        auto current_scene = RenderWidget::RendererToScene(iren->FindPokedRenderer(x, y));
        if(current_scene == nullptr){
            return;
        }
        QSlider* slider = nullptr;
        if(current_scene == axial_scene_){
            slider = axial_toolbar_->slice_slider;
        }else if(current_scene == coronal_scene_){
            slider = coronal_toolbar_->slice_slider;
        }else if(current_scene == sagittal_scene_){
            slider = sagittal_toolbar_->slice_slider;
        }else{
            return;
        }
        int new_value = slider->value();
        if(e_id == vtkCommand::MouseWheelForwardEvent){
            new_value -= 1;
        }else{
            new_value += 1;
        }
        slider->setValue(new_value);
        return;
    }
    //neglected other event handling...
}

void MainWidget::InteractorEventCallback(vtkObject* caller, unsigned long e_id, void* client_data, void* call_data)
{
    auto mw = static_cast<MainWidget*>(client_data);
    mw->handleInteractorEvent(caller, e_id);
}

void MainWidget::resetPlaneView()
{
    if(orthoplanes_ == nullptr){
        return;
    }
    auto scene_plane = orthoplanes_->getAxialPlane();
    auto plane_width = scene_plane->getSize1();
    auto plane_height = scene_plane->getSize2();
    auto plane_display_size = axial_scene_->getDisplaySize();
    auto w_h_aspect = (double)plane_display_size[0] / (double)plane_display_size[1];
    double parallel_scale = std::max(plane_height / 2, plane_width / (2 * w_h_aspect)) + 0.01;
//     SPDLOG_DEBUG("plane width:{} plane height:{} display width:{} display height:{} aspect:{}",
//         plane_width, plane_height, plane_display_size[0], plane_display_size[1], w_h_aspect, parallel_scale);
    axial_scene_->getActiveCamera()->SetParallelScale(parallel_scale);

    scene_plane = orthoplanes_->getCoronalPlane();
    plane_width = scene_plane->getSize1();
    plane_height = scene_plane->getSize2();
    plane_display_size = coronal_scene_->getDisplaySize();
    w_h_aspect = (double)plane_display_size[0] / (double)plane_display_size[1];
    parallel_scale = std::max(plane_height / 2, plane_width / (2 * w_h_aspect)) + 0.01;
//     SPDLOG_DEBUG("plane width:{} plane height:{} display width:{} display height:{} aspect:{}",
//         plane_width, plane_height, plane_display_size[0], plane_display_size[1], w_h_aspect, parallel_scale);
    coronal_scene_->getActiveCamera()->SetParallelScale(parallel_scale);

    scene_plane = orthoplanes_->getSagittalPlane();
    plane_width = scene_plane->getSize1();
    plane_height = scene_plane->getSize2();
    plane_display_size = sagittal_scene_->getDisplaySize();
    w_h_aspect = (double)plane_display_size[0] / (double)plane_display_size[1];
    parallel_scale = std::max(plane_height / 2, plane_width / (2 * w_h_aspect)) + 0.01;
//     SPDLOG_DEBUG("plane width:{} plane height:{} display width:{} display height:{} aspect:{}",
//         plane_width, plane_height, plane_display_size[0], plane_display_size[1], w_h_aspect, parallel_scale);
    sagittal_scene_->getActiveCamera()->SetParallelScale(parallel_scale);

    if(volume_data_){
        auto exts = volume_data_->GetExtent();
        auto spacings = volume_data_->GetSpacing();
        double vol_height = (exts[4] - exts[1]) * spacings[1];
        double vol_width = (exts[3] - exts[0]) * spacings[0];
        plane_display_size = ortho_scene_->getDisplaySize();
        w_h_aspect = (double)plane_display_size[0] / (double)plane_display_size[1];
        parallel_scale = std::max(plane_height / 2, plane_width / (2 * w_h_aspect)) + 0.1;
        ortho_scene_->getActiveCamera()->SetParallelScale(parallel_scale);
    }

    axial_scene_->render();
}

void MainWidget::createWLWLabels()
{
    auto create_scene_label = [](Scene* s){
        auto new_label = std::make_shared<LabelSprite>();
        new_label->setFontSize(18);
        new_label->setPosition(0.01, 0.90);
        new_label->connectScene(*s);
        return new_label;
    };

    auto lookup_table = orthoplanes_->getLookupTable(0);
    double min_v = lookup_table->GetTableRange()[0];
    double max_v = lookup_table->GetTableRange()[1];
    double current_wl = (int)((max_v + min_v) / 2);
    double current_ww = max_v - min_v;

    scene_wlw_labels_.clear();

    auto new_label = create_scene_label(axial_scene_);
    std::string wlw_text = std::format("WL:{} WW:{}", current_wl, current_ww);
    new_label->setText(wlw_text.c_str());
    scene_wlw_labels_.insert({axial_scene_, new_label});

    new_label = create_scene_label(coronal_scene_);
    new_label->setText(wlw_text.c_str());
    scene_wlw_labels_.insert({coronal_scene_, new_label});

    new_label = create_scene_label(sagittal_scene_);
    new_label->setText(wlw_text.c_str());
    scene_wlw_labels_.insert({sagittal_scene_, new_label});

    auto surface_volume = SurfaceVolumeSprite::SafeDownCast(ortho_scene_->getSprite("Volume"));
    if(surface_volume){
        auto current_threshold_range = surface_volume->getThresholdRange();
        double min_v = current_threshold_range[0];
        double max_v = current_threshold_range[1];            
        double current_wl = (int)((max_v + min_v) / 2);
        double current_ww = max_v - min_v;
        wlw_text = std::format("WL:{} WW:{}", current_wl, current_ww);
    }else{
        wlw_text = std::format("WL:-- WW:--", current_wl, current_ww);
    }
    new_label = create_scene_label(ortho_scene_);
    scene_wlw_labels_.insert({ortho_scene_, new_label});
    new_label->setText(wlw_text.c_str());
}

void MainWidget::loadPlugins()
{
    pm_.unloadAll();
    for(const auto& rd : pm_.records()){
        if(rd.iid == IDICOM_READER_IID){
            continue;
        }
        auto objs = pm_.loadAllByIID(rd.iid);
        if(objs.empty()){
            SPDLOG_WARN("Load plugin:\"{}\" failed!", rd.iid.toUtf8().toStdString());
            continue;
        }
        auto plugin_viewer = qobject_cast<IViewer*>(objs[0]);
        if(plugin_viewer == nullptr){
            continue;
        }
        if(rd.iid == IPATH_DESIGNER_IID){
            auto pd = qobject_cast<IPathDesigner*>(objs[0]);
            if(pd == nullptr){
                continue;
            }
            pd->loadPaths(gp_);
        }
        auto sw_sourcer = qobject_cast<ISceneWidgetSource*>(objs[0]);
        if(sw_sourcer){
            sw_sourcer->setRenderWidget(ui_.render_widget);
        }
        if(!plugin_viewer->createView(ui_.tabWidget)){
            continue;
        }
        auto vw = plugin_viewer->getViewWidget();
        if(vw){
            ui_.tabWidget->addTab(vw, rd.locale_display_name);
        }
    }    
}

void MainWidget::scanPluginDirectory()
{
    const QString plugin_root_dir = GetPluginDirectory();
    if(!QFileInfo(plugin_root_dir).exists()){
        SPDLOG_WARN("Plugin dir:{} not exists!", plugin_root_dir.toUtf8().toStdString());
        return;
    }
    pm_.addSearchPath(plugin_root_dir);
    pm_.scan();
    pm_.dump();
}

bool MainWidget::initUI()
{    
    auto extents = volume_data_->GetExtent();
    Arry2i axial_slice_range = {extents[4], extents[5]};
    Arry2i coronal_slice_range = {extents[2], extents[3]};
    Arry2i sagittal_slice_range = {extents[0], extents[1]};

    if(axial_scene_ == nullptr){
        axial_scene_ = new AxialPlaneScene(Axial);
        ui_.render_widget->addScene(*axial_scene_);
        axial_scene_->setBkgClr(Color::Gray);
    }
    if(axial_toolbar_ == nullptr){
        axial_toolbar_ = new SliceOverlayToolbar(ui_.render_widget);
        axial_toolbar_->maximize_btn->setProperty("Scene", QString::fromStdString(std::string(axial_scene_->name())));
        connect(axial_toolbar_->slice_slider, &QSlider::valueChanged, this, &MainWidget::slotAxialSlicePositionChanged);
        connect(axial_toolbar_->maximize_btn, &QAbstractButton::toggled, this, &MainWidget::slotMaximizeButtonClicked);
    }    
    axial_toolbar_->slice_slider->setRange(axial_slice_range[0], axial_slice_range[1]);
    
    if(coronal_scene_ == nullptr){
        coronal_scene_ = new CoronalPlaneScene(Coronal);
        ui_.render_widget->addScene(*coronal_scene_);   
        coronal_scene_->setBkgClr(Color::Gray);
    }    
    if(coronal_toolbar_ == nullptr){
        coronal_toolbar_ = new SliceOverlayToolbar(ui_.render_widget);
        coronal_toolbar_->maximize_btn->setProperty("Scene", QString::fromStdString(std::string(coronal_scene_->name())));
        connect(coronal_toolbar_->slice_slider, &QSlider::valueChanged, this, &MainWidget::slotCoronalSlicePositionChanged);
        connect(coronal_toolbar_->maximize_btn, &QAbstractButton::toggled, this, &MainWidget::slotMaximizeButtonClicked);
    }    
    coronal_toolbar_->slice_slider->setRange(coronal_slice_range[0], coronal_slice_range[1]);    

    if(sagittal_scene_ == nullptr){
        sagittal_scene_ = new SagittalPlaneScene(Sagittal);
        ui_.render_widget->addScene(*sagittal_scene_);
        sagittal_scene_->setBkgClr(Color::Gray);
    }
    if(sagittal_toolbar_ == nullptr){
        sagittal_toolbar_ = new SliceOverlayToolbar(ui_.render_widget);
        sagittal_toolbar_->maximize_btn->setProperty("Scene", QString::fromStdString(std::string(sagittal_scene_->name())));
        connect(sagittal_toolbar_->slice_slider, &QSlider::valueChanged, this, &MainWidget::slotSagittalSlicePositionChanged);
        connect(sagittal_toolbar_->maximize_btn, &QAbstractButton::toggled, this, &MainWidget::slotMaximizeButtonClicked);
    }    
    sagittal_toolbar_->slice_slider->setRange(sagittal_slice_range[0], sagittal_slice_range[1]);    

    if(ortho_scene_ == nullptr){
        ortho_scene_ = new Scene(Ortho);
        ui_.render_widget->addScene(*ortho_scene_);
        ortho_scene_->setBkgClr(Color::Silver);
    }
    if(ortho_toolbar_ == nullptr){
        ortho_toolbar_ = new OrthoToolbar(ui_.render_widget);
        ortho_toolbar_->maximize_btn->setProperty("Scene", QString::fromStdString(std::string(ortho_scene_->name())));
        connect(ortho_toolbar_->maximize_btn, &QAbstractButton::toggled, this, &MainWidget::slotMaximizeButtonClicked);
    }
    
    if(wheel_event_cb_cmd_ == nullptr){
        wheel_event_cb_cmd_ = vtkSmartPointer<vtkCallbackCommand>::New();
        wheel_event_cb_cmd_->SetCallback(InteractorEventCallback);
        wheel_event_cb_cmd_->SetClientData(this);
        ui_.render_widget->interactor()->AddObserver(vtkCommand::MouseWheelForwardEvent, wheel_event_cb_cmd_);
        ui_.render_widget->interactor()->AddObserver(vtkCommand::MouseWheelBackwardEvent, wheel_event_cb_cmd_);
    }
        
    show();    
    update();

    initModels(volume_data_);

    ui_.lookup_table_cb->setCurrentIndex(0);
    ui_.render_mode_cb->setCurrentIndex(1);    

    Point3 ortho_center = volume_data_->GetCenter();
    orthoplanes_->setOrthoCenter(ortho_center);

    auto spacings = volume_data_->GetSpacing();
    auto slider_pos_value = (int)(orthoplanes_->getAxialPlane()->getPosition() / spacings[2]);
    axial_toolbar_->slice_slider->setValue(slider_pos_value);
    slider_pos_value = (int)(orthoplanes_->getCoronalPlane()->getPosition() / spacings[1]);
    coronal_toolbar_->slice_slider->setValue(slider_pos_value);
    slider_pos_value = (int)(orthoplanes_->getSagittalPlane()->getPosition() / spacings[0]);
    sagittal_toolbar_->slice_slider->setValue(slider_pos_value);

    ui_.show_volume_cb->setChecked(true);

    ortho_scene_->resetView();
    axial_scene_->resetView();

    ui_.render_widget->render();

    loadPlugins();

    //{        
    //    if(!ui_.render_widget->interactor()->HasObserver(vtkCommand::WindowResizeEvent)){
    //        auto cb = vtkSmartPointer<vtkCallbackCommand>::New();
    //        cb->SetCallback([](vtkObject* caller, unsigned long, void* clientData, void*){
    //                auto* self = static_cast<MainWidget*>(clientData);
    //                auto* iren = static_cast<vtkRenderWindowInteractor*>(caller);
    //                self->onVtkWindowResizeEvent(iren);
    //        });
    //        cb->SetClientData(this);
    //        ui_.render_widget->interactor()->AddObserver(vtkCommand::WindowResizeEvent, cb);
    //    }        
    //}

    updateSceneViewport();

    createOrientationMarkerWidgets();
    ui_.orientation_marker_cb->setChecked(true);

    createWLWLabels();

    QTimer::singleShot(500, [this](){
        SPDLOG_DEBUG("Delay timer timeout!");
        updateOverlayPos();
        //resetPlaneView();
    });

    return true;
}

void MainWidget::resizeEvent(QResizeEvent *e)
{
    SPDLOG_DEBUG("resizeEvent called!");
    QMainWindow::resizeEvent(e);
    if(axial_scene_){
        updateOverlayPos();
    }    
}

void MainWidget::updateOverlayPos()
{
    SPDLOG_INFO("updateOverlayPos called!");

    int w = ui_.render_widget->width(), h = ui_.render_widget->height();

    QRect new_geo;
    auto vp = axial_scene_->getRenderer()->GetViewport();     
    new_geo.setX(vp[0] * w);
    new_geo.setY((1.0 - vp[3]) * h);
    new_geo.setWidth((vp[2] - vp[0]) * w);
    new_geo.setHeight(kSceneToolbarHeight);
    axial_toolbar_->setGeometry(new_geo);
    axial_toolbar_->raise();

    vp = coronal_scene_->getRenderer()->GetViewport();     
    new_geo.setX(vp[0] * w);
    new_geo.setY((1.0 - vp[3]) * h);
    new_geo.setWidth((vp[2] - vp[0]) * w);
    new_geo.setHeight(kSceneToolbarHeight);
    coronal_toolbar_->setGeometry(new_geo);
    coronal_toolbar_->raise();    

    vp = sagittal_scene_->getRenderer()->GetViewport();     
    new_geo.setX(vp[0] * w);
    new_geo.setY((1.0 - vp[3]) * h);
    new_geo.setWidth((vp[2] - vp[0]) * w);
    new_geo.setHeight(kSceneToolbarHeight);
    sagittal_toolbar_->setGeometry(new_geo);
    sagittal_toolbar_->raise();      

    vp = ortho_scene_->getRenderer()->GetViewport();  
    new_geo.setX(vp[0] * w);
    new_geo.setY((1.0 - vp[3]) * h);
    new_geo.setWidth((vp[2] - vp[0]) * w);
    new_geo.setHeight(kSceneToolbarHeight);
    ortho_toolbar_->setGeometry(new_geo);
    ortho_toolbar_->raise();

    resetPlaneView();
}

void MainWidget::onVtkWindowResizeEvent(vtkRenderWindowInteractor* interactor)
{    
    auto s = interactor->GetRenderWindow()->GetSize();
    auto width = s[0], height = s[1];
    SPDLOG_INFO("onVtkWindowResizeEvent:{},{}", width, height);
}

void MainWidget::updateSceneViewport()
{
    coronal_scene_->setViewport(kBorder, 2 * kBorder + kNormalSceneHeight, kBorder + kNormalSceneWidth, 1.0 - kBorder);
    coronal_scene_->resetView();
    axial_scene_->setViewport(2 * kBorder + kNormalSceneWidth, kBorder * 2 + kNormalSceneHeight, 1.0 - kBorder, 1.0 - kBorder);    
    axial_scene_->resetView();
    sagittal_scene_->setViewport(kBorder, kBorder, kBorder + kNormalSceneWidth, kBorder + kNormalSceneHeight);    
    sagittal_scene_->resetView();
    ortho_scene_->setViewport(kBorder * 2.0 + kNormalSceneWidth, kBorder, 1.0 - kBorder, kBorder + kNormalSceneHeight);
    ortho_scene_->resetView();
    auto ortho_camera = ortho_scene_->getActiveCamera();
    ortho_camera->SetFocalPoint(0.0, 0.0, 0.0);
    ortho_camera->SetPosition(0.0, -200.0, 0.0);
    //ortho_camera->SetViewUp(0.0, 1.0, 0.0);
    ortho_scene_->getRenderer()->ResetCameraClippingRange();
    ortho_camera->OrthogonalizeViewUp();
    ortho_scene_->getActiveCamera()->Modified();
    //Point3 p = ortho_camera->GetPosition();
    //Point3 f = ortho_camera->GetFocalPoint();
    //Point3 v = ortho_camera->GetViewUp();

    //SPDLOG_DEBUG("OrthoCameraPos:{} focal:{} viewup:{}", p.toStr(), f.toStr(), v.toStr());
    ui_.render_widget->render();
}

Scene* MainWidget::maximizeSceneViewport(const std::string& scene_name)
{
    auto old_flag = vtkObject::GetGlobalWarningDisplay();
    vtkObject::GlobalWarningDisplayOff();
    Scene* maximize_scene = nullptr;
    std::array<Scene*, 4> all_scenes = {coronal_scene_, axial_scene_, sagittal_scene_, ortho_scene_};
    for(auto i = 0; i < 4; ++i){
        auto s = all_scenes[i];
        if(s->name() == scene_name){
            s->setViewport(kBorder, kBorder, 1.0 - kBorder, 1.0 - kBorder);
            s->resetView();
            orientation_marker_widgets_[i]->GetOrientationMarker()->SetVisibility(1);
            orientation_marker_widgets_[i]->Modified();
            maximize_scene = s;
        }else{
            s->setViewport(0.0, 0.0, 0.0001, 0.0001);
            orientation_marker_widgets_[i]->GetOrientationMarker()->SetVisibility(0);
        }
    }
    ui_.render_widget->render();
    vtkObject::SetGlobalWarningDisplay(old_flag);
    return maximize_scene;
}

void MainWidget::restoreSceneViewport()
{
    updateSceneViewport();
    //std::array<Scene*, 4> all_scenes = {coronal_scene_, axial_scene_, sagittal_scene_, ortho_scene_};
    for(auto i = 0; i < 4; ++i){
        orientation_marker_widgets_[i]->GetOrientationMarker()->SetVisibility(1);
        orientation_marker_widgets_[i]->Modified();
    }
    ui_.render_widget->render();
}

void MainWidget::maximizeToolbarPos(QPushButton* event_button, Scene* s)
{
    auto vp = s->getRenderer()->GetViewport();
    int w = ui_.render_widget->width(), h = ui_.render_widget->height();
    std::array<QWidget*, 4> all_slice_bars = {coronal_toolbar_, axial_toolbar_, sagittal_toolbar_, ortho_toolbar_};
    QRect new_geo;
    for(auto i = 0; i < 4; ++i){
        QPushButton* max_btn = nullptr;
        if(i == 3){
            max_btn = qobject_cast<OrthoToolbar*>(all_slice_bars[i])->maximize_btn;
        }else{
            max_btn = qobject_cast<SliceOverlayToolbar*>(all_slice_bars[i])->maximize_btn;
        }
        if(max_btn == event_button){
            new_geo.setX(vp[0] * w);
            new_geo.setY((1.0 - vp[3]) * h);
            new_geo.setWidth((vp[2] - vp[0]) * w);
            new_geo.setHeight(kSceneToolbarHeight);
            all_slice_bars[i]->setGeometry(new_geo);
            all_slice_bars[i]->raise();
        }else{
            all_slice_bars[i]->setGeometry(0, 0, 0, 0);
        }
    }
}

void MainWidget::restoreToolbarPos()
{
    updateOverlayPos();
}
