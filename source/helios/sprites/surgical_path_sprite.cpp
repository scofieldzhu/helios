/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/9/9
*******************************************************/
#include "surgical_path_sprite.h"
#include <vtkCylinderSource.h>
#include <vtkConeSource.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkCellPicker.h>
#include <vtkProp3DCollection.h>
#include <vtkAssemblyPath.h>
#include "mirfak/core/scene.h"
#include "mirfak/sprites/sphere_sprite.h"
#include "mirfak/basic/log_service.h"

namespace {
    constexpr double kDefaultConeLength = 2.0;
    constexpr double kDefaultCylinderLength = 14.0;
    constexpr double kDefaultPathLength = kDefaultConeLength + kDefaultCylinderLength;
}

MIRFAK_NAMESPACE_BEGIN

SurgicalPathSprite::SurgicalPathSprite(double radius)
    :cylinder_source_(vtkSmartPointer<vtkCylinderSource>::New()),
    cone_source_(vtkSmartPointer<vtkConeSource>::New()),
    polys_appender_(vtkSmartPointer<vtkAppendPolyData>::New()),
    clean_polys_(vtkSmartPointer<vtkCleanPolyData>::New()),
    start_point_({0.0, 0.0, 0.0}),
    end_point_({0.0, kDefaultPathLength, 0.0}),
    start_ctrl_sphere_(std::make_unique<SphereSprite>()),
    end_ctrl_sphere_(std::make_unique<SphereSprite>())
{
    cylinder_source_->SetRadius(radius);
    cylinder_source_->SetResolution(50);
    cylinder_source_->SetCenter(0.0, kDefaultCylinderLength / 2.0, 0.0);
    cylinder_source_->SetHeight(kDefaultCylinderLength);
    cone_source_->SetRadius(radius);
    cone_source_->SetDirection(0.0, 1.0, 0.0); //keep consistent with default direction of cylinder!
    cone_source_->SetResolution(50);
    cone_source_->SetHeight(kDefaultConeLength);
    cone_source_->SetCenter(0.0, kDefaultCylinderLength + kDefaultConeLength / 2.0, 0.0);
    start_ctrl_sphere_->setCenter(start_point_);
    start_ctrl_sphere_->setColor(Color::Green);
    start_ctrl_sphere_->setRadius(0.2);
    end_ctrl_sphere_->setCenter(end_point_);
    end_ctrl_sphere_->setColor(Color::Green);
    end_ctrl_sphere_->setRadius(0.2);
    start_ctrl_sphere_->setVisibility(false);
    addChild(*start_ctrl_sphere_);
    end_ctrl_sphere_->setVisibility(false);
    addChild(*end_ctrl_sphere_);
    boundbox_points_[0] = {-radius, kDefaultPathLength, 0.0}; //left-top point
    boundbox_points_[1] = end_point_; //middle-top point
    boundbox_points_[2] = {radius, kDefaultPathLength, 0.0}; //right-top point
    boundbox_points_[3] = {-radius, 0.0, 0.0}; //left-bottom point
    boundbox_points_[4] = start_point_; //middle-bottom point
    boundbox_points_[5] = {radius, 0.0, 0.0}; //right-bottom point
}

SurgicalPathSprite::~SurgicalPathSprite()
{
}

double SurgicalPathSprite::getMinimumPathLength() const
{
    return kDefaultConeLength + 0.1;
}

void SurgicalPathSprite::setCustomTailPolys(vtkPolyData *tail_polys, double length)
{
    
}

void SurgicalPathSprite::setStartPoint(const Point3& pt)
{
    SPDLOG_DEBUG("setStartPoint called!");
    const double new_length = pt.distanceTo(end_point_);
    if(new_length < getMinimumPathLength()){
        SPDLOG_ERROR("Too less length:{} between start and end point! min value:{}", new_length, getMinimumPathLength());
        return;
    }
    start_point_ = pt;
    setLength(new_length);
    auto trans = calcTransform(start_point_, end_point_);
    getTransform()->SetMatrix(trans->GetMatrix());
    modified();
}

void SurgicalPathSprite::setEndPoint(const Point3& pt)
{
    SPDLOG_DEBUG("setEndPoint called!");
    const double new_length = pt.distanceTo(start_point_);
    if(new_length < getMinimumPathLength()){
        SPDLOG_ERROR("Too less length:{} between start and end point! min value:{}", new_length, getMinimumPathLength());
        return;
    }
    end_point_= pt;
    setLength(new_length);
    auto trans = calcTransform(start_point_, end_point_);
    getTransform()->SetMatrix(trans->GetMatrix());
    modified();
}

void SurgicalPathSprite::setPoints(const Point3& s_pt, const Point3& e_pt)
{
    const double new_length = s_pt.distanceTo(e_pt);
    if(new_length < getMinimumPathLength()){
        SPDLOG_ERROR("Too less length:{} between start and end point! min value:{}", new_length, getMinimumPathLength());
        return;
    }
    start_point_ = s_pt;
    end_point_ = e_pt;
    setLength(new_length);
    auto trans = calcTransform(start_point_, end_point_);
    getTransform()->SetMatrix(trans->GetMatrix());
    modified();
}

double SurgicalPathSprite::getLength() const
{
    return start_point_.distanceTo(end_point_);
}

double SurgicalPathSprite::getRadius()const
{
    return cone_source_->GetRadius();
}

vtkAlgorithmOutput* SurgicalPathSprite::getPolyData() const
{
    return polys_appender_->GetOutputPort();
}

void SurgicalPathSprite::startEditing(const Point2& pt)
{
    __super__::startEditing(pt);
    start_ctrl_sphere_->setVisibility(true);
    end_ctrl_sphere_->setVisibility(true);
    modified();
}

void SurgicalPathSprite::endEditing(const Point2& pt)
{
    __super__::endEditing(pt);
    start_ctrl_sphere_->setVisibility(false);
    end_ctrl_sphere_->setVisibility(false);
    modified();
}

vtkSmartPointer<vtkTransform> SurgicalPathSprite::calcTransform(const Point3& start_pt, const Point3& end_pt) const
{
    const Vec3 origin_dir = {0.0, 1.0, 0.0};
    Vec3 new_dir = end_pt - start_pt;
    new_dir.normalize();
    Vec3 ortho_dir = origin_dir.cross(new_dir);
    auto s = ortho_dir.norm();
    auto c = origin_dir.dot(new_dir);
    auto trans = vtkSmartPointer<vtkTransform>::New();
    trans->Identity();
    trans->PostMultiply();
    if(IsZero(s)){
        // parallel or reverse direction
        if(c < 0.0){
            //inverse direction
            trans->RotateWXYZ(180.0, 0.0, 0.0, 1.0);
        }
        //same direction
    }else{
        ortho_dir.normalize();
        double angle = RadianToDegree(std::atan2(s, c));
        trans->RotateWXYZ(angle, ortho_dir);
    }
    trans->Translate(start_pt);
    return trans;
}

Pt3Arry6 SurgicalPathSprite::getBoundboxWorldPoints() const
{
    Pt3Arry6 points;
    for(auto i = 0; i < 6; ++i){
        points[i] = getTransform()->TransformPoint(boundbox_points_[i]);
    }
    return points;
}

void SurgicalPathSprite::setLength(double length)
{
    double body_length = length - kDefaultConeLength;
    cylinder_source_->SetCenter(0.0, body_length / 2.0, 0.0);
    cylinder_source_->SetHeight(body_length);  
    cylinder_source_->Modified();
    cone_source_->SetCenter(0.0, body_length + kDefaultConeLength / 2.0, 0.0);    
    cone_source_->Modified();    
    end_ctrl_sphere_->setCenter({0.0, length, 0.0});
    polys_appender_->Modified();

    double radius = getRadius();
    boundbox_points_[0] = {-radius, length, 0.0}; //left-top point
    boundbox_points_[1] = {0.0, length, 0.0}; //middle-top point
    boundbox_points_[2] = {radius, length, 0.0}; //right-top point
}

void SurgicalPathSprite::createSource(Scene &scene)
{
    polys_appender_->AddInputConnection(cylinder_source_->GetOutputPort());
    polys_appender_->AddInputConnection(cone_source_->GetOutputPort());
    clean_polys_->SetInputConnection(polys_appender_->GetOutputPort());
}

void SurgicalPathSprite::makeActors(Scene &scene)
{
    vtkNew<vtkPolyDataMapper> m;
    m->SetInputConnection(clean_polys_->GetOutputPort());
    vtkNew<vtkActor> a;
    a->SetMapper(m);
    //a->GetProperty()->SetOpacity(0.5);
    addSceneProp(scene, a, "path");
}

int SurgicalPathSprite::computeEditState(Scene& s, int x, int y, int modify /*= 0*/)
{
    edit_state_ = ES_NONE;
    if(!isEditing() || !getVisibility(s)){        
        return edit_state_;
    }
    edit_state_ = ES_OUTSIDE;
    auto path_actor = getSceneActor("path", &s);
    auto start_sphere_actor = start_ctrl_sphere_->getFirstSceneActor(&s);
    auto end_sphere_actor  = end_ctrl_sphere_->getFirstSceneActor(&s);
    if(path_actor == nullptr || start_ctrl_sphere_ == nullptr || end_sphere_actor == nullptr){
        return edit_state_;
    }
    if(!picker_dict_.contains(&s)){
        auto new_picker = PickerPtr::New();
        new_picker->SetPickFromList(true);
        new_picker->AddPickList(path_actor);
        new_picker->AddPickList(start_sphere_actor);
        new_picker->AddPickList(end_sphere_actor);
        picker_dict_.insert({&s, new_picker});
    }
    auto target_picker = picker_dict_[&s];
    if(!target_picker->Pick(x, y, 0.0, s.getRenderer())){
        return edit_state_;
    }
    auto get_top_actor = [](vtkAssemblyPath* path){
        auto top_node = path->GetFirstNode();
        auto top_actor = vtkActor::SafeDownCast(top_node->GetViewProp());
        if(top_actor){
            return top_actor;
        }
        auto next_node = path->GetNextNode();
        while(next_node){
            if(auto a = vtkActor::SafeDownCast (next_node->GetViewProp ())){
                return a;
            }
            next_node = path->GetNextNode();
        }
        return (vtkActor*)(nullptr);
    };
    auto picked_top_actor = get_top_actor(target_picker->GetPath());
    if(picked_top_actor == nullptr){
        return edit_state_;
    }
    if(picked_top_actor == path_actor){
        edit_state_ = ES_HOVER;
        return edit_state_;
    }
    if(picked_top_actor == start_sphere_actor){
        edit_state_ = ES_START_POINT_SELECTED;
        return edit_state_;
    }
    if(picked_top_actor == end_sphere_actor){
        edit_state_ = ES_END_POINT_SELECTED;
        return edit_state_;
    }
    return edit_state_;
}

NAMESPACE_END
