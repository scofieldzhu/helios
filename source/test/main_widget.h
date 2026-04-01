#ifndef __main_widget_h__
#define __main_widget_h__

#include <QMainWindow>
#include "ui_main_widget.h"
#include "slice_overlay_toolbar.h"
#include "ortho_toolbar.h"
#include "helios/core/scene.h"
#include "helios/core/render_widget.h"
#include <vtkSmartPointer.h>
#include "raycasting_conf.h"
#include "helios/sprites/wlw_editor.h"
#include "helios/sprites/surgical_path_editor.h"
#include "helios/appbase/plugin_manager.h"

class vtkImageData;
class vtkVolumeProperty;
class vtkOrientationMarkerWidget2;

namespace helios{
    class VolumeSprite;
    class SurfaceVolumeSprite;
    class OrthoPlanesSprite;
    class PlaneIntersectionLineSprite;
    class PlaneIntersectionLineEditor;
    class PolygonEditor;
    class PolygonSprite;
    class LabelSprite;
    class TorusSprite;
    class ArrowSprite;
    class PointMoveToolSprite;
    class PointMoveToolEditor;
}

enum VolumeRenderMode{
    VRM_COMPOSITION,
    VRM_SURFACE,
};

class IDICOMReader;

class MainWidget : public QMainWindow
{
    Q_OBJECT

public:
    bool initUI();
    VolumeRenderMode getVolumeRenderMode()const;
    void setGlobalImage(vtkSmartPointer<vtkImageData> d);
    MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

signals:
    void slicePositionWalk(QString name, int);
    void globalImageUpdated();

private slots:
    void slotShowPointCloudBtnClicked(bool chk);
    void slotShowVolumeBtnClicked(bool chk);
    void slotCurrentRenderModeChanged(const QString& name);
    void slotCurrentLookupTableChanged(const QString& name);
    void slotShowOrthoPlanesBtnClicked(bool chk);
    void slotMprCbClicked(bool chk);
    //void slotPointSizeReturnPressed();
    void slotMaximizeButtonClicked(bool chk);
    void slotAxialSlicePositionChanged(int);
    void slotCoronalSlicePositionChanged(int);
    void slotSagittalSlicePositionChanged(int);
    void slotDrawPolygonButtonClicked(bool chk = false);
    void slotDeletePolygonButtonPressed();
    void slotInterpolateButtonToggled(bool chk);
    void slotShowOrientationMarker(bool chk);
    void slotAdjustWLW(bool chk);
    void slotOpacitySliderValueChanged(int);
    void slotLoadDicomBtnPressed();
    void slotGlobalImageUpdated();

private:
    static void OnWLWUpdating(vtkObject* caller, unsigned long eid, void* client_data, void* call_data);
    void forceApplyCurrentRaycatingConfig();
    void applyRaycastingConfig(const QString& name, vtkVolumeProperty* vol_prop);
    void setThresholdRange(int lower, int high);
    void resizeEvent(QResizeEvent* e) override;
    void updateOverlayPos();
    void onVtkWindowResizeEvent(vtkRenderWindowInteractor* interactor);
    void updateSceneViewport();
    helios::Scene* maximizeSceneViewport(const std::string& scene_name);
    void restoreSceneViewport();
    void maximizeToolbarPos(QPushButton* event_button, helios::Scene* s);
    void restoreToolbarPos();
    void createVolumeSprite(vtkImageData* vol_data);  
    void initModels(vtkImageData* data);    
    void onAxialSlicePositionChanged(helios::SlicePlaneSprite* plane, double);
    void onAxialSliceRangeChanged(helios::SlicePlaneSprite* plane, double);
    void onCoronalSlicePositionChanged(helios::SlicePlaneSprite* plane, double);
    void onCoronalSliceRangeChanged(helios::SlicePlaneSprite* plane, double);
    void onSagittalSlicePositionChanged(helios::SlicePlaneSprite* plane, double);
    void onSagittalSliceRangeChanged(helios::SlicePlaneSprite* plane, double);
    void createOrientationMarkerWidgets();
    void onWLWUpdated(helios::Scene* scene, double wl, double ww);
    void handleInteractorEvent(vtkObject* caller, unsigned long e_id);
    static void InteractorEventCallback(vtkObject* caller, unsigned long e_id, void* client_data, void* call_data);
    void resetPlaneView();
    void createWLWLabels();
    void loadPlugins();
    void scanPluginDirectory();
    void onImageReadFinished(IDICOMReader* reader);
    Ui::MainWidget ui_;
    vtkSmartPointer<vtkImageData> volume_data_;    
    helios::Scene* axial_scene_ = nullptr;
    SliceOverlayToolbar* axial_toolbar_ = nullptr;
    helios::Scene* coronal_scene_ = nullptr;
    SliceOverlayToolbar* coronal_toolbar_ = nullptr;
    helios::Scene* sagittal_scene_ = nullptr;
    SliceOverlayToolbar* sagittal_toolbar_ = nullptr;
    helios::OrthoPlanesSprite* orthoplanes_ = nullptr;
    std::vector<std::unique_ptr<helios::PlaneIntersectionLineSprite>> crosslines_;
    helios::Scene* ortho_scene_ = nullptr;
    OrthoToolbar* ortho_toolbar_ = nullptr;
    helios::PlaneIntersectionLineEditor* axial_pil_editor_ = nullptr;
    helios::PlaneIntersectionLineEditor* coronal_pil_editor_ = nullptr;
    helios::PlaneIntersectionLineEditor* sagittal_pil_editor_ = nullptr;
    std::unique_ptr<helios::PolygonSprite> polygon_;
    vtkSmartPointer<helios::PolygonEditor> polygon_editor_;
    RaycastingConfGroup sys_raycasting_conf_gp_;
    vtkSmartPointer<vtkOrientationMarkerWidget2> orientation_marker_widgets_[4];
    vtkSmartPointer<helios::WLWEditor> change_threshold_range_editor_;
    std::map<helios::Scene*, std::shared_ptr<helios::LabelSprite>> scene_wlw_labels_;
    helios::PluginManager pm_;
    helios::SurgicalPathGroup gp_;
    vtkSmartPointer<vtkCallbackCommand> wheel_event_cb_cmd_;
};

#endif