/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/12/17
*******************************************************/
#include "sagittal_plane_scene.h"
#include <vtkCamera.h>
#include "slice_plane_sprite.h"

MIRFAK_NAMESPACE_BEGIN

SagittalPlaneScene::SagittalPlaneScene(const std::string_view& name)
	:PlaneScene(name)
{

}

SagittalPlaneScene::~SagittalPlaneScene()
{

}

void SagittalPlaneScene::handleStartRenderEvent()
{
	if(plane_ == nullptr || !hasSprite(*plane_)){
		return;
	}
	auto active_camera = renderer_->GetActiveCamera();
	auto focal_point = plane_->getTransformedCenter();
	active_camera->SetFocalPoint(focal_point);
	auto camera_viewup = plane_->getTransformedViewUp2Vector();
	camera_viewup.normalize();
	active_camera->SetViewUp(camera_viewup);
	auto proj_normal = plane_->getTransformedNormal();
	proj_normal.normalize();
	auto camera_position = focal_point + proj_normal * 10;
	active_camera->SetPosition(camera_position);
	active_camera->SetClippingRange(9, 11);
	active_camera->OrthogonalizeViewUp();
	modified();
}

NAMESPACE_END