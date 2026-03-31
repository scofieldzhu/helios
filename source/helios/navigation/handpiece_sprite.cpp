/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/9/12
*******************************************************/
#include "handpiece_sprite.h"
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkActor.h>
#include <vtkTransform.h>
#include <vtkSTLReader.h>
#include <vtkTrackerTool.h>
#include <vtkLandmarkTransform.h>
#include "mirfak/basic/log_service.h"
#include "device_administrator.h"
#include "drill_config_repository.h"

MIRFAK_NAMESPACE_BEGIN

extern std::string QStrToStd(const QString& qstr);

namespace{
    const Point3 kOriginMarkPoint1(0.0, 5.0, 0.0);
    const Point3 kOriginMarkPoint2(0.0, 0.0, 0.0);
    const Point3 kOriginMarkPoint3(0.0, 0.0, 5.0);
}

HandpieceSprite::HandpieceSprite()
    :source_model_polydata_(vtkSmartPointer<vtkPolyData>::New()),
    transform_data_filter_(vtkSmartPointer<vtkTransformPolyDataFilter>::New()),
    drill_(std::make_unique<DrillSprite>()),
    current_mark_point1_(kOriginMarkPoint1),
    current_mark_point2_(kOriginMarkPoint2),
    current_mark_point3_(kOriginMarkPoint3)
{
    vtkNew<vtkTransform> t;
    t->Identity();
    transform_data_filter_->SetTransform(t);    
    makePolyDataEmpty(source_model_polydata_);
    addChild(*drill_);
}

HandpieceSprite::~HandpieceSprite()
{
}

void HandpieceSprite::createSource(Scene& scene)
{
    transform_data_filter_->SetInputData(source_model_polydata_);
}

void HandpieceSprite::makeActors(Scene& scene)
{
    vtkNew<vtkPolyDataMapper> m;
    m->SetInputConnection(transform_data_filter_->GetOutputPort());
    vtkNew<vtkActor> a;
    a->SetMapper(m);
    addSceneProp(scene, a, "tool");
}

void HandpieceSprite::switchDrill(const QString& drill_name)
{
    auto config_opt = DrillConfigRepository::GetInst().getDrillConfig(drill_name);
    if(!config_opt){
        SPDLOG_ERROR("Drill named:\"{}\" not exists!", QStrToStd(drill_name));
        return;
    }
    drill_->loadConfig(*config_opt);
    moveToBitDrill(config_opt->length);
    auto tip_offset = calcDrillTipOffset(*config_opt);
    updateToolCalibrationMatrix(tip_offset);
    modified();
}

DrillSprite* HandpieceSprite::getDrill()
{
    return drill_.get();
}

void HandpieceSprite::makePolyDataEmpty(vtkPolyData *data)
{
    data->Reset();
    data->SetPoints(vtkSmartPointer<vtkPoints>::New());    
    data->SetVerts(vtkSmartPointer<vtkCellArray>::New());
    data->SetLines(vtkSmartPointer<vtkCellArray>::New());
    data->SetPolys(vtkSmartPointer<vtkCellArray>::New());
    data->SetStrips(vtkSmartPointer<vtkCellArray>::New());
}

bool HandpieceSprite::bindTrackerTool(const QString& tool_name)
{
    auto active_tracker = DeviceAdministrator::GetInst().getDefaultTracker();
    if(active_tracker == nullptr){
        return false;
    }
    auto config_opt = active_tracker->config().getToolConfig(tool_name);
    if(!config_opt || config_opt->type != TrackerToolType::kLocator){
        return false;
    }
    if(!config_opt->isAxisCalibrated()){
        SPDLOG_ERROR("The axis normal of tool:\"{}\" is not calibrated yet!", QStrToStd(tool_name));
        return false;
    }
    bound_tool_config_opt_ = config_opt;
    updateSourceModelData();
    calib_drill_config_opt_ = DrillConfigRepository::GetInst().getDrillConfig(*bound_tool_config_opt_->calib_drill);
    //switch to calibration drill automatically
    switchDrill(*bound_tool_config_opt_->calib_drill); 
    return true;
}

void HandpieceSprite::updateSourceModelData()
{
    source_model_polydata_ = readModelData(*bound_tool_config_opt_->model_filepath);
    transform_data_filter_->SetInputData(source_model_polydata_);
    source_model_polydata_->Modified();
    modified();
}

Point3 HandpieceSprite::calcDrillTipOffset(const DrillConfig& drill_conf) const
{    
    auto seed_drill_axis_normal = bound_tool_config_opt_->getCalibAxisNormal();
    auto len_diff = drill_conf.length - calib_drill_config_opt_->length;
    return *bound_tool_config_opt_->tip_point + (*seed_drill_axis_normal) * len_diff;
}

vtkSmartPointer<vtkPolyData> HandpieceSprite::readModelData(const QString& filename) const
{
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(filename.toUtf8().data());
    reader->Update();
    vtkSmartPointer<vtkPolyData> polydata = reader->GetOutput();
    if(polydata == nullptr){
        SPDLOG_ERROR("read model file:{} failed!", QStrToStd(filename));
        return nullptr;
    }
    return polydata;
}

void HandpieceSprite::moveToBitDrill(double drill_length)
{
    vtkNew<vtkTransform> t;
    t->Identity();    
    t->Translate(0.0, 0.0, drill_length);    
    transform_data_filter_->SetTransform(t);
    transform_data_filter_->Modified();
}

void HandpieceSprite::updateToolCalibrationMatrix(const Point3& tip_offset)
{
    auto tracker_tool = DeviceAdministrator::GetInst().getDefaultTracker()->getTrackerTool(bound_tool_config_opt_->name);
    assert(tracker_tool);
    /*
    * 在VTK中，vtkMatrix4x4 采用 列向量右乘 约定，因此：矩阵的前三列，分别表示“新坐标系的 X / Y / Z 轴在旧坐标系中的方向”，第四列是平移。
    * 新坐标轴X：为旧坐标系的-Y方向
    * 新坐标轴Y：为旧坐标系的-Z方向
    * 新坐标轴Z：为旧坐标系的+X方向
    */
    const double matrix_values[16] =
    {
        0.0,  0.0,  1.0, tip_offset[0],
        -1.0, 0.0,  0.0, tip_offset[1],
        0.0,  -1.0, 0.0, tip_offset[2],
        0.0,  0.0,  0.0, 1.0
    };
    auto current_tip_calib_matrix = tracker_tool->GetCalibrationMatrix();
    current_tip_calib_matrix->DeepCopy(matrix_values);

    const double kReprAxisLen = 10.0;
    const double kReprHandLen = 10.0;
    Point3 locator_pt1(0.0, 0.0, 0.0);
    Point3 locator_pt2(0.0, 0.0, -kReprAxisLen);
    Point3 locator_pt3(0.0, -kReprHandLen, 0.0);

    Point3 target_pt1 = tip_offset;
    const Vec3 kDrillAxisNormal = bound_tool_config_opt_->getCalibAxisNormal().value();
    Point3 target_pt2 = target_pt1 + kDrillAxisNormal * kReprAxisLen;
    Point3 origin_pt(0.0, 0.0, 0.0);
    auto proj_pt = Line(tip_offset, kDrillAxisNormal).projectPoint(origin_pt);
    Vec3 hand_axis_normal = (origin_pt - proj_pt).normalized();
    Point3 target_pt3 = target_pt1 + hand_axis_normal * kReprHandLen;
    
    vtkNew<vtkTransform> new_trans; // 表示新构建的尖端坐标系到工具坐标系的变换矩阵
    new_trans->SetMatrix(current_tip_calib_matrix);
    target_pt1 = new_trans->GetInverse()->TransformPoint(target_pt1);
    target_pt2 = new_trans->GetInverse()->TransformPoint(target_pt2);
    target_pt3 = new_trans->GetInverse()->TransformPoint(target_pt3);

    vtkNew<vtkPoints> source_pts;
    source_pts->InsertNextPoint(locator_pt1);
    source_pts->InsertNextPoint(locator_pt2);
    source_pts->InsertNextPoint(locator_pt3);
    vtkNew<vtkPoints> target_pts;
    target_pts->InsertNextPoint(target_pt1);
    target_pts->InsertNextPoint(target_pt2);
    target_pts->InsertNextPoint(target_pt3);
    vtkNew<vtkLandmarkTransform> rotate_trans;
    rotate_trans->SetSourceLandmarks(source_pts);
    rotate_trans->SetTargetLandmarks(target_pts);
    rotate_trans->SetModeToRigidBody();
    rotate_trans->Update();

    new_trans->PreMultiply();
    new_trans->Concatenate(rotate_trans->GetMatrix());
    tracker_tool->SetCalibrationMatrix(new_trans->GetMatrix());
}

void HandpieceSprite::updateTransform()
{
    if(!bound_tool_config_opt_){ //no tracker tool attached
        return;
    }
    auto tracker_tool = DeviceAdministrator::GetInst().getDefaultTracker()->getTrackerTool(bound_tool_config_opt_->name);
    assert(tracker_tool);
    vtkNew<vtkPoints> source_pts;
    source_pts->InsertNextPoint(current_mark_point1_);
    source_pts->InsertNextPoint(current_mark_point2_);
    source_pts->InsertNextPoint(current_mark_point3_);
    vtkNew<vtkPoints> target_pts;
    Point3 new_pt1 = tracker_tool->GetTransform()->TransformPoint(kOriginMarkPoint1);
    target_pts->InsertNextPoint(new_pt1);
    Point3 new_pt2 = tracker_tool->GetTransform()->TransformPoint(kOriginMarkPoint2);
    target_pts->InsertNextPoint(new_pt2);
    Point3 new_pt3 = tracker_tool->GetTransform()->TransformPoint(kOriginMarkPoint3);
    target_pts->InsertNextPoint(new_pt3);
    vtkNew<vtkLandmarkTransform> new_trans;
    new_trans->SetSourceLandmarks(source_pts);
    new_trans->SetTargetLandmarks(target_pts);
    new_trans->SetModeToRigidBody();
    new_trans->Update();
    getTransform()->PostMultiply();
    getTransform()->Concatenate(new_trans->GetMatrix());
    current_mark_point1_ = new_pt1;
    current_mark_point2_ = new_pt2;
    current_mark_point3_ = new_pt3;
    modified();
}

std::optional<QString> HandpieceSprite::getBoundTrackerTool() const
{
    if(bound_tool_config_opt_){
        return bound_tool_config_opt_->name;
    }
    return std::nullopt;
}

NAMESPACE_END


