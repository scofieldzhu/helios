/*******************************************************
* author: scofieldzhu
* time:2025/8/26
*******************************************************/
#include "abstract_volume_sprite.h"
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkImageData.h>
#include <vtkVolumeProperty.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkRenderer.h>
#include <vtkCommand.h>
#include <vtkPlaneCollection.h>
#include "helios/basic/log_service.h"
#include "helios/core/scene.h"

HELIOS_NAMESPACE_BEGIN

AbstractVolumeSprite::AbstractVolumeSprite(vtkIdType bytes)
	:raycast_mapper_(vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New()),
    clipping_plane_(vtkSmartPointer<vtkPlane>::New())
{
    raycast_mapper_->ReleaseDataFlagOn();
    raycast_mapper_->SetMaxMemoryInBytes(bytes);
    clipping_plane_->SetOrigin(0.0, 0.0, 0.0);
    clipping_plane_->SetNormal(1.0, 0.0, 0.0);	
}

AbstractVolumeSprite::~AbstractVolumeSprite()
{
    if(start_render_event_id_ != -1){
        event_renderer_->RemoveObserver(start_render_event_id_);
        start_render_event_id_ = -1;
        event_renderer_ = nullptr;
    }
}

void AbstractVolumeSprite::setInputData(vtkImageData* input)
{
    if(input == nullptr){
        SPDLOG_ERROR("Null input data pointer passed!");
        return;
    }
	raycast_mapper_->SetInputData(input);
	modified();
}

vtkImageData* AbstractVolumeSprite::getInput()const
{
    if(raycast_mapper_->GetInputCount() == 0){
        return nullptr;
    }
    return vtkImageData::SafeDownCast(raycast_mapper_->GetInput());
}

vtkImageData* AbstractVolumeSprite::maskImageData()const
{
	return vtkImageData::SafeDownCast(raycast_mapper_->GetMaskInput());
}

void AbstractVolumeSprite::setMaskImageData(vtkImageData* data)
{
    if(data == nullptr){
        SPDLOG_ERROR("Null mask input data pointer passed!");
        return;
    }
    raycast_mapper_->SetMaskTypeToBinary();
    raycast_mapper_->SetMaskInput(data);
    raycast_mapper_->Modified();
    modified();
}

bool AbstractVolumeSprite::addToScene(Scene& scene)
{
    //only one scene can be connected to this object!
    if(existsDisplayScene()){
        SPDLOG_ERROR("I have been connected a scene object already!");
        return false;
    }
    start_render_event_id_ = scene.getRenderer()->AddObserver(vtkCommand::StartEvent, this, &AbstractVolumeSprite::onStartEventNotify);
    event_renderer_ = scene.getRenderer();
    return Sprite::addToScene(scene);
}

void AbstractVolumeSprite::onStartEventNotify(vtkObject* caller, unsigned long event_id, void* data)
{
    renderTime()->Modified();
}

void AbstractVolumeSprite::removeFromScene(Scene& scene)
{
    Sprite::removeFromScene(scene);
    if(start_render_event_id_ != -1){
        scene.getRenderer()->RemoveObserver(start_render_event_id_);
        start_render_event_id_ = -1;
        event_renderer_ = nullptr;
    }
}

bool AbstractVolumeSprite::hasChangedSince(unsigned long since_mtime)const
{
    if(Sprite::hasChangedSince(since_mtime)){
        return true;
    }
    auto input = getInput();
    if(input && input->GetMTime() > since_mtime){
        return true;
    }
    auto property = getVolumeProperty();
    if(property == nullptr){
        return false;
    }
    auto clr_func = property->GetRGBTransferFunction();
    auto opacity_func = property->GetScalarOpacity();
    if((clr_func && property->GetMTime() > since_mtime) || (opacity_func && opacity_func->GetMTime() > since_mtime)){
        return true;
    }
    return false;
}

void AbstractVolumeSprite::turnClippingPlaneOn()
{
    raycast_mapper_->AddClippingPlane(clipping_plane_);
}

void AbstractVolumeSprite::turnClippingPlaneOff()
{
    raycast_mapper_->RemoveClippingPlane(clipping_plane_);
}

bool AbstractVolumeSprite::isClippingPlaneOn()const
{
    if(raycast_mapper_->GetClippingPlanes()){
        return raycast_mapper_->GetClippingPlanes()->GetNumberOfItems();
    }
    return false;    
}

void AbstractVolumeSprite::updateClippingPlane(const Point3& origin, const Vec3& normal)
{
    clipping_plane_->SetOrigin(origin);
    clipping_plane_->SetNormal(normal);
    clipping_plane_->Modified();
    raycast_mapper_->Modified();
    modified();
}

NAMESPACE_END