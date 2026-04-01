/******************************************************** 
* author: scofieldzhu
* time:2025/12/17
*******************************************************/
#include "axial_plane_scene.h"
#include <vtkCamera.h>
#include "slice_plane_sprite.h"
#include "helios/basic/log_service.h"

HELIOS_NAMESPACE_BEGIN

AxialPlaneScene::AxialPlaneScene(const std::string_view& name)
	:PlaneScene(name)
{

}

AxialPlaneScene::~AxialPlaneScene()
{
	
}

void AxialPlaneScene::handleStartRenderEvent()
{
	if(plane_ == nullptr || !hasSprite(*plane_)){
		return;
	}
	auto active_camera = renderer_->GetActiveCamera();
	auto focal_point = plane_->getTransformedCenter();
	active_camera->SetFocalPoint(focal_point);
	auto camera_viewup = plane_->getTransformedViewUp2Vector();
	camera_viewup.normalize();
	camera_viewup = -camera_viewup;
	active_camera->SetViewUp(camera_viewup);
	auto proj_normal = plane_->getTransformedNormal();
	proj_normal.normalize();
	auto camera_position = focal_point - proj_normal * 10;	
	active_camera->SetPosition(camera_position);
	active_camera->SetClippingRange(9, 11);
	active_camera->OrthogonalizeViewUp();
	//SPDLOG_DEBUG("Current active camera information:\n {}", VtkObjToLogStr(active_camera));
	modified();
}

NAMESPACE_END