/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/5/15
*******************************************************/
#include "plane_sprite.h"
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include "mirfak/basic/log_service.h"

MIRFAK_NAMESPACE_BEGIN

PlaneSprite::PlaneSprite()
    :plane_source_(vtkSmartPointer<vtkPlaneSource>::New())
{
}

PlaneSprite::~PlaneSprite()
{

}

void PlaneSprite::setPoints(const Point3& origin, const Point3& pt1, const Point3& pt2)
{
    plane_source_->SetOrigin(origin);
    plane_source_->SetPoint1(pt1[0], pt1[1], pt1[2]);
    plane_source_->SetPoint2(pt2[0], pt2[1], pt2[2]);
    plane_source_->Update();
    modified();
}

void PlaneSprite::makeActors(Scene& scene)
{
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(plane_source_->GetOutputPort());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    addSceneProp(scene, actor, "plane");
}

NAMESPACE_END


