/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/12/11
*******************************************************/
#include "plane_scene.h"
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include "slice_plane_sprite.h"
#include "render_widget.h"

HELIOS_NAMESPACE_BEGIN

PlaneScene::PlaneScene(const std::string_view& name)
	:Scene(name)
{

}

PlaneScene::~PlaneScene()
{
	//if(scene_widget_ && start_event_tag_ != -1){//remove start event callback
	//	scene_widget_->renderWindow()->RemoveObserver(start_event_tag_);
	//}
	//start_event_tag_ = -1;
}

void PlaneScene::setPlane(SlicePlaneSprite* plane)
{
	plane_ = plane;
	//handleStartRenderEvent();
	modified_time_->Modified();
	modified();
}

void PlaneScene::handleStartRenderEvent()
{
	//TODO: 
}

void PlaneScene::addToRenderWidget(RenderWidget& sw)
{
	__super__::addToRenderWidget(sw);
	//add start render event callback
	vtkNew<vtkCallbackCommand> cb;
	cb->SetCallback(PlaneScene::StartRenderEventCallback);
	cb->SetClientData(this);
	start_event_tag_ = scene_widget_->renderWindow()->AddObserver(vtkCommand::StartEvent, cb);
}

void PlaneScene::StartRenderEventCallback(vtkObject* caller, unsigned long id, void* client_data, void*)
{
	auto scene = static_cast<PlaneScene*>(client_data);
	if(scene){
		scene->handleStartRenderEvent();
	}	
}

void PlaneScene::removeFromRenderWidget()
{
	if(scene_widget_ && start_event_tag_ != -1){//remove start event callback
		scene_widget_->renderWindow()->RemoveObserver(start_event_tag_);
	}
	start_event_tag_ = -1;
	__super__::removeFromRenderWidget();
}

void PlaneScene::fitViewToPlane()
{
	if(plane_ == nullptr){
		return;
	}	
	double vp[4]= {0.0};
	renderer_->GetViewport(vp);
	auto display_size = renderer_->GetRenderWindow()->GetSize();
	auto w = std::max(1.0, (vp[2] - vp[0]) * display_size[0]);
	auto h = std::max(1.0, (vp[2] - vp[0]) * display_size[1]);
	auto aspect = w / h;
	auto plane_width  = plane_->getSize1();
	auto plane_height = plane_->getSize2();
	auto s = std::min(plane_height / 2.0, plane_width / (2.0 * aspect));
	getActiveCamera()->SetParallelScale(s);
	modified();
}

NAMESPACE_END
