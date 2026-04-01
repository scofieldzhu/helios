/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/7/10
*******************************************************/
#include "sphere_sprite.h"
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>

HELIOS_NAMESPACE_BEGIN

SphereSprite::SphereSprite()
    :sphere_source_(vtkSmartPointer<vtkSphereSource>::New())
{
    sphere_source_->SetPhiResolution(100);
    sphere_source_->SetThetaResolution(100);
}

SphereSprite::~SphereSprite()
{
}

void SphereSprite::createSource(Scene &scene)
{
}

void SphereSprite::makeActors(Scene &scene)
{
    vtkNew<vtkPolyDataMapper> m;
    m->SetInputConnection(sphere_source_->GetOutputPort());    
    vtkNew<vtkActor> a;
    a->SetMapper(m);
    addSceneProp(scene, a, "sphere");
}

void SphereSprite::setRadius(double r)
{
    sphere_source_->SetRadius(r);
    sphere_source_->Update();
    modified();
}

double SphereSprite::getRadius() const
{
    return sphere_source_->GetRadius();
}

void SphereSprite::setCenter(const Point3& pt)
{
    sphere_source_->SetCenter(pt);
    sphere_source_->Modified();
    modified();
}

Point3 SphereSprite::getCenter()const
{
    return sphere_source_->GetCenter();
}

NAMESPACE_END
