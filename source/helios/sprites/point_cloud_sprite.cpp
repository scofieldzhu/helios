/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/5/15
*******************************************************/
#include "point_cloud_sprite.h"
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkCellArray.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include "polygon_sprite.h"

HELIOS_NAMESPACE_BEGIN

PointCloudSprite::PointCloudSprite(PointType type)
    :polydata_(vtkSmartPointer<vtkPolyData>::New()),
    point_type_(type)
{
    if(point_type_ == PT_VERTEX){
        point_size_ = 1.0f;
        vgly_filter_ = vtkSmartPointer<vtkVertexGlyphFilter>::New();
    }else{
        point_size_ = 0.1f;
        sphere_point_ = vtkSmartPointer<vtkSphereSource>::New();
        glyph_source_ = vtkSmartPointer<vtkGlyph3D>::New();
    }
}

PointCloudSprite::~PointCloudSprite()
{
}

vtkActor* PointCloudSprite::getCloudActor(Scene& s) const
{
    return getSceneActor("points", &s);
}

void PointCloudSprite::setPointSize(float size)
{
    if(point_type_ == PT_SPHERE){
        sphere_point_->SetRadius(size);
        sphere_point_->Update();
    }else{
        for(auto s : getDisplaySceneList()){
            auto actor = getSceneActor("points", s);
            actor->GetProperty()->SetPointSize(size);
            actor->Modified();
        }
    }
    point_size_ = size;
    modified();
}

void PointCloudSprite::setPoints(vtkPoints* pts)
{
    if(pts == nullptr){
        return;
    }
    polydata_->Reset();
    polydata_->SetPoints(pts);
    if(point_type_ == PT_SPHERE){
        //glyph_source_->Modified();
    }
    modified();
}

void PointCloudSprite::setPoints(const Pt3List& pts)
{
    vtkNew<vtkPoints> points;
    for(const auto& pt : pts){
        points->InsertNextPoint(pt[0], pt[1], pt[2]);
    }
    setPoints(points);
}

Pt3List PointCloudSprite::getPoints() const
{
    Pt3List result_pts;
    auto pts = polydata_->GetPoints();
    for(auto i = 0; i < pts->GetNumberOfPoints(); ++i){
        auto pt = pts->GetPoint(i);
        result_pts.push_back({pt[0], pt[1], pt[2]});
    }
    return result_pts;
}

void PointCloudSprite::createSource(Scene& scene)
{
}

void PointCloudSprite::makeActors(Scene& scene)
{
    vtkNew<vtkPolyDataMapper> mapper;
    if(point_type_ == PT_SPHERE){
        sphere_point_->SetRadius(point_size_); 
        sphere_point_->SetPhiResolution(8); 
        sphere_point_->SetThetaResolution(8); 
        sphere_point_->Update();  
        glyph_source_->SetSourceConnection(sphere_point_->GetOutputPort());  
        glyph_source_->SetInputData(polydata_);  
        glyph_source_->Update();  
        mapper->SetInputConnection(glyph_source_->GetOutputPort());
    }else if(point_type_ == PT_VERTEX){
        vgly_filter_->SetInputData(polydata_);  
        vgly_filter_->Update();  
        mapper->SetInputConnection(vgly_filter_->GetOutputPort());
    }
    vtkNew<vtkActor> cloud_actor;
    cloud_actor->SetMapper(mapper);
    if(point_type_ == PT_VERTEX){
        cloud_actor->GetProperty()->SetRepresentationToPoints(); 
        cloud_actor->GetProperty()->SetPointSize(point_size_);
    }
    addSceneProp(scene, cloud_actor, "points");
}

NAMESPACE_END
