/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/4/21
*******************************************************/
#include "plane_cutter_sprite.h"
#include <vtkPlaneSource.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkProperty.h>
#include <vtkCleanPolyData.h>
#include <vtkTransform.h>
#include "mirfak/basic/log_service.h"

MIRFAK_NAMESPACE_BEGIN

PlaneCutterSprite::PlaneCutterSprite()
    :clean_filter_(vtkSmartPointer<vtkCleanPolyData>::New()),
    cutter_(vtkSmartPointer<vtkCutter>::New())
{
}

PlaneCutterSprite::~PlaneCutterSprite()
{}

// void PlaneCutterSprite::updatePosition(const Point3& pos)
// {
//     Point3 fixed_pos = boundplanes_.limitPoint(pos);
//     Point3 fixed_pos_proj;
//     double d[3]; 
//     if(extrusion_direction_ == ED_CUT_PLANE_NORMAL){
//         vtkPlane::ProjectPoint(fixed_pos.data(), plane_source_->GetOrigin(), plane_source_->GetNormal(), d);
//     }else{
//         vtkPlane::ProjectPoint(fixed_pos.data(), cut_plane_->GetOrigin(), cut_plane_->GetNormal(), d);    
//     }      
//     fixed_pos_proj = d;  
//     double dist = fixed_pos.distanceTo(fixed_pos_proj);    
//     Vec3 direction = fixed_pos - fixed_pos_proj;
//     direction.normalize();
//     Vec3 lenvec = direction * dist;    
//     Point3 cur_center = plane_source_->GetCenter();
//     Point3 new_center = cur_center + lenvec;    
//     plane_source_->SetCenter(new_center[0], new_center[1], new_center[2]);    
//     modified();

//     //broadcasterHelper::Broadcast(PlaneCutterPosChangedEvent(this));
// }

// double PlaneCutterSprite::getCurrentDistance(CuboidPlanes::PlaneType planetype) const
// {
//     Point3 cur_centerpt = plane_source_->GetCenter();
//     return boundplanes_.getPlane(planetype).calcDistance(cur_centerpt);
// }

// int32_t PlaneCutterSprite::getCurrentStepNum(double steplen, CuboidPlanes::PlaneType planetype)const
// {    
//     return getCurrentDistance(planetype) / steplen;
// }

void PlaneCutterSprite::setLineWidth(double w)
{
    line_width_ = w;
}

bool PlaneCutterSprite::hasChangedSince(unsigned long since_mtime) const
{
    if(Sprite::hasChangedSince(since_mtime)){
        return true;
    }
    return plane_source_->GetMTime() > since_mtime;
}

void PlaneCutterSprite::setTargetPlaneSource(vtkSmartPointer<vtkPlaneSource> plane_source)
{
    plane_source_ = plane_source;
}

void PlaneCutterSprite::setCutPlane(vtkSmartPointer<vtkPlane> cut_plane)
{
    cut_plane_ = cut_plane;
}

std::optional<PlaneCutterSprite::PointCouple> PlaneCutterSprite::getLinePoints() const
{
    PointCouple couple;
    clean_filter_->Update();
    auto poly_data = clean_filter_->GetOutput();
    if(poly_data->GetNumberOfPoints() >= 2){
        couple[0] = poly_data->GetPoints()->GetPoint(0);
        couple[1] = poly_data->GetPoints()->GetPoint(1);
        return couple;
    }
    return std::nullopt;
}

void PlaneCutterSprite::createSource(Scene& pane)
{
    cutter_->SetInputConnection(plane_source_->GetOutputPort());
    cutter_->SetCutFunction(cut_plane_);   
    clean_filter_->SetInputConnection(cutter_->GetOutputPort()); 
    clean_filter_->Update();
}

void PlaneCutterSprite::makeActors(Scene& scene)
{
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(cutter_->GetOutputPort());    
    mapper->SetResolveCoincidentTopologyToPolygonOffset();
    mapper->SetResolveCoincidentTopologyPolygonOffsetParameters(-1, -1);
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->LightingOff(); //avoid wrong color according to camera light
    addSceneProp(scene, actor.Get(), "PlaneCutter");
    actor->GetProperty()->SetLineWidth(line_width_);
}

NAMESPACE_END