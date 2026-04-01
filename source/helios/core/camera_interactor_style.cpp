/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/5/8
*******************************************************/
#include "camera_interactor_style.h"
#include <vtkObjectFactory.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include "plane_scene.h"
#include "render_widget.h"
#include "slice_plane_sprite.h"
#include "helios/basic/log_service.h"

HELIOS_NAMESPACE_BEGIN

vtkStandardNewMacro(CameraInteractorStyle);

int GetRendererPlaneMode(vtkRenderer* ren)
{
    auto scene = RenderWidget::RendererToScene(ren);
    if(scene){
        return scene->isA(PlaneScene::GetClsTypeName()) ? 1 : 0; 
    }
    return -1;
}

CameraInteractorStyle::CameraInteractorStyle()
    : vtkInteractorStyleTrackballCamera()
{
}

CameraInteractorStyle::~CameraInteractorStyle()
{
}

void CameraInteractorStyle::OnLeftButtonDown()
{
    left_button_down_ = true;    
}

void CameraInteractorStyle::OnLeftButtonUp()
{
    left_button_down_ = false;
}

void CameraInteractorStyle::OnRightButtonDown()
{
    if(!interactor_enabled_){
        return;
    }
    if(!left_button_down_){
        this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
        if(this->CurrentRenderer == nullptr){
            return;
        }
        this->GrabFocus(this->EventCallbackCommand);
        auto plane_mode = GetRendererPlaneMode(this->CurrentRenderer);
        if(plane_mode < 0){
            return;
        }
        target_event_renderer_ = this->CurrentRenderer;
        if(plane_mode){
            StartDolly();            
        }else{            
            StartRotate();
        }        
    }	
    right_button_down_ = true;
}

void CameraInteractorStyle::OnRightButtonUp()
{    
    if(!interactor_enabled_){
        return;
    }
    if(this->CurrentRenderer == nullptr){
        return;
    }
    vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    right_button_down_ = false;
}

void CameraInteractorStyle::OnMiddleButtonDown()
{
    vtkInteractorStyleTrackballCamera::OnMiddleButtonDown();
    middle_button_down_ = true;
    if(this->CurrentRenderer == nullptr){
        return;
    }
    target_event_renderer_ = this->CurrentRenderer;
}

void CameraInteractorStyle::OnMiddleButtonUp()
{
    vtkInteractorStyleTrackballCamera::OnMiddleButtonUp();
    middle_button_down_ = false;
}

void CameraInteractorStyle::Pan()
{
    if(!interactor_enabled_){
        return;
    }
    if(this->CurrentRenderer == nullptr || target_event_renderer_ != this->CurrentRenderer){
        return;
    }    
    auto new_event_point = this->Interactor->GetEventPosition();
    auto old_event_point = this->Interactor->GetLastEventPosition();
    auto ren_size = CurrentRenderer->GetSize();
    auto move_x = double(old_event_point[0] - new_event_point[0]) / ren_size[0] * 2;
    auto move_y = double(old_event_point[1] - new_event_point[1]) / ren_size[1] * 2;
    vtkCamera* camera = this->CurrentRenderer->GetActiveCamera(); 
    auto win_center = camera->GetWindowCenter();
    camera->SetWindowCenter(win_center[0] + move_x, win_center[1] + move_y);
    if(this->Interactor->GetLightFollowCamera()){
        this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }
    this->Interactor->Render();
}

void CameraInteractorStyle::Dolly(double factor)
{
    if(!interactor_enabled_){
        return;
    }
    if(this->CurrentRenderer == nullptr || this->CurrentRenderer != this->target_event_renderer_){
        return;
    }
    auto plane_mode = GetRendererPlaneMode(this->CurrentRenderer);
    if(plane_mode < 0){
        return;
    }
    if(plane_mode){        
        Dolly2D(factor);
        return;
    }
    if(current_scale_factor_ < 0){
        cacheCurrentScale();
    }
    auto event_point = this->Interactor->GetEventPosition();
    double point_in_world[4];
    this->ComputeDisplayToWorld(event_point[0], event_point[1], 0, point_in_world);
    auto camera = this->CurrentRenderer->GetActiveCamera();
    if(camera->GetParallelProjection()){
        double scale = camera->GetParallelScale() / factor;
        scale = std::max(current_scale_factor_ / max_scale_factor_, scale);
        scale = std::min(current_scale_factor_ / min_scale_factor_, scale);
        camera->SetParallelScale(scale);
    }else{
        double dist = camera->GetDistance();
        double new_dist = dist / factor;
        new_dist = std::max(current_scale_factor_ / max_scale_factor_, new_dist);
        new_dist = std::min(current_scale_factor_ / min_scale_factor_, new_dist);
        factor = dist / new_dist;
        camera->Dolly(factor);
        if(this->AutoAdjustCameraClippingRange){
            this->CurrentRenderer->ResetCameraClippingRange();
        }
    }
    double new_event_point[3];
    this->ComputeWorldToDisplay(point_in_world[0], point_in_world[1], point_in_world[2], new_event_point);
    auto ren_size = CurrentRenderer->GetSize();
    auto move_x = double(new_event_point[0] - event_point[0]) / ren_size[0] * 2;
    auto move_y = double(new_event_point[1] - event_point[1]) / ren_size[1] * 2;
    auto win_center = camera->GetWindowCenter();
    camera->SetWindowCenter(win_center[0] + move_x, win_center[1] + move_y);
    if(this->Interactor->GetLightFollowCamera()){
        this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }
    this->Interactor->Render();
}

void CameraInteractorStyle::Dolly2D(double factor)
{
    if(this->CurrentRenderer == nullptr){
		return;
	}
	auto camera = this->CurrentRenderer->GetActiveCamera();
	if(camera->GetParallelProjection()){
		camera->SetParallelScale(camera->GetParallelScale() / factor);
	}else{
		camera->Dolly(factor);
		if(this->AutoAdjustCameraClippingRange){
			this->CurrentRenderer->ResetCameraClippingRange();
		}
	}
	if(this->Interactor->GetLightFollowCamera()){
		this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
	}
	this->Interactor->Render();
}

void CameraInteractorStyle::OnMouseMove()
{
    if((right_button_down_ || middle_button_down_) && left_button_down_){
        return;
    }
    int event_x = this->Interactor->GetEventPosition()[0];
    int event_y = this->Interactor->GetEventPosition()[1];
    this->FindPokedRenderer(event_x, event_y);
    if(this->CurrentRenderer == nullptr){
        return;
    }
    if(right_button_down_ || middle_button_down_){
        vtkInteractorStyleTrackballCamera::OnMouseMove();
        return;
    }
    if(!free_move_pick_enabled_){
        return;
    }
    auto current_scene = RenderWidget::RendererToScene(this->CurrentRenderer);
    if(current_scene == nullptr){
        return;
    }
    current_free_moving_data_.x = event_x;
    current_free_moving_data_.y = event_y;
    current_free_moving_data_.event_scene = dynamic_cast<PlaneScene*>(current_scene);
    if(last_move_renderer_ != this->CurrentRenderer){
        current_free_moving_data_.scalar = std::nullopt;
    }else{
        current_free_moving_data_.scalar = pickImageScalar(event_x, event_y, current_free_moving_data_.event_scene);
    }
    this->InvokeEvent(MouseFreeMoving, &current_free_moving_data_);
    last_move_renderer_ = this->CurrentRenderer;
}

void CameraInteractorStyle::OnChar()
{
}

void CameraInteractorStyle::cacheCurrentScale()
{
    current_scale_factor_ = -1;
    if(this->CurrentRenderer == nullptr){
        return;
    }
    auto camera = this->CurrentRenderer->GetActiveCamera();
    if(camera->GetParallelProjection()){
        current_scale_factor_ = camera->GetParallelScale();
    }else{
        current_scale_factor_ = camera->GetDistance();
    }
}

void CameraInteractorStyle::setMinMaxScale(double minScaleFactor, double maxScaleFactor)
{
    min_scale_factor_ = minScaleFactor;
    max_scale_factor_ = maxScaleFactor;
}

void CameraInteractorStyle::setInteractorEnable(bool enable)
{
    interactor_enabled_ = enable;
}

void CameraInteractorStyle::OnMouseWheelForward()
{
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
    if(this->CurrentRenderer == nullptr){
        return;
    }
    auto plane_mode = GetRendererPlaneMode(this->CurrentRenderer);
    if(plane_mode < 0){
        return;
    }
    if(!plane_mode){
        vtkInteractorStyleTrackballCamera::OnMouseWheelForward();
    }    
}

void CameraInteractorStyle::OnMouseWheelBackward()
{
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
    if(this->CurrentRenderer == nullptr){
        return;
    }
    auto plane_mode = GetRendererPlaneMode(this->CurrentRenderer);
    if(plane_mode < 0){
        return;
    }
    if(!plane_mode){
        vtkInteractorStyleTrackballCamera::OnMouseWheelBackward();
    }   
}

std::optional<double> CameraInteractorStyle::pickImageScalar(int x, int y, PlaneScene* s) const
{    
    if(s == nullptr){
        return std::nullopt;
    }
    auto attached_plane = s->plane();
    if(attached_plane == nullptr){
        return std::nullopt;
    }
    Point3 rayline_p1 = s->displayToWorld({static_cast<double>(x), static_cast<double>(y), 0.0});
    Point3 rayline_p2 = s->displayToWorld({static_cast<double>(x), static_cast<double>(y), 1.0});    
    Plane p(attached_plane->getTransformedCenter(), attached_plane->getTransformedNormal());
    auto intersection_pt_opt = p.intersectWithLine(Line::FromTwoPoints(rayline_p1, rayline_p2));
    if(!intersection_pt_opt){
        return std::nullopt;
    }
    auto image_data = attached_plane->getInputData();
    if(image_data == nullptr){
        return std::nullopt;
    }
    Point3 picked_world_point = *intersection_pt_opt;
    double ijk[3];
    image_data->TransformPhysicalPointToContinuousIndex(picked_world_point, ijk);
    ijk[0] = vtkMath::Round(ijk[0]);
    ijk[1] = vtkMath::Round(ijk[1]);
    ijk[2] = vtkMath::Round(ijk[2]);
    int extent[6];
    image_data->GetExtent(extent);
    if(ijk[0] < extent[0] || ijk[0] > extent[1] ||
       ijk[1] < extent[2] || ijk[1] > extent[3] ||
       ijk[2] < extent[4] || ijk[2] > extent[5]){
        return std::nullopt;
    }
    return image_data->GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], 0);
}

void CameraInteractorStyle::OnLeave()
{
    vtkInteractorStyleTrackballCamera::OnLeave();

    if(free_move_pick_enabled_){
        current_free_moving_data_.x = 0;
        current_free_moving_data_.y = 0;
        current_free_moving_data_.event_scene = nullptr;
        current_free_moving_data_.scalar = std::nullopt;
        this->InvokeEvent(MouseFreeMoving, &current_free_moving_data_);
        last_move_renderer_ = nullptr;
    }
}

void CameraInteractorStyle::enableFreeMovePick(bool enabled)
{
    free_move_pick_enabled_ = enabled;
}

NAMESPACE_END
