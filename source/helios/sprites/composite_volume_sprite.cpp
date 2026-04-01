/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/5/15
*******************************************************/
#include "composite_volume_sprite.h"
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkImageData.h>
#include <vtkVolumeProperty.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkLODProp3D.h>
#include <vtkCullerCollection.h>
#include <vtkRenderer.h>
#include <vtkCuller.h>
#include <vtkFrustumCoverageCuller.h>
#include <vtkVolumePicker.h>
#include "helios/basic/log_service.h"
#include "helios/core/scene.h"

HELIOS_NAMESPACE_BEGIN

CompositeVolumeSprite::CompositeVolumeSprite(vtkIdType max_mem_bytes)
	:AbstractVolumeSprite(max_mem_bytes),
	volume_prop3d_(vtkSmartPointer<vtkLODProp3D>::New()),
    vol_picker_(vtkSmartPointer<vtkVolumePicker>::New())
{
	vtkNew<vtkVolumeProperty> vol_property;
	vol_property->SetInterpolationTypeToLinear();
	raycast_mapper_->AutoAdjustSampleDistancesOff();
    raycast_mapper_->SetBlendModeToComposite();
    raycast_mapper_->UseJitteringOn();
    opengl_ray_id_ = volume_prop3d_->AddLOD(raycast_mapper_, vol_property, 0.0);
    volume_prop3d_->SetLODLevel(opengl_ray_id_, 1.0);
    setVolumePropertyToDefault(0);
    volume_prop3d_->EnableLOD(opengl_ray_id_);

    vol_picker_->PickFromListOn();
    vol_picker_->AddPickList(volume_prop3d_);
    vol_picker_->SetVolumeOpacityIsovalue(0.1);
}

CompositeVolumeSprite::~CompositeVolumeSprite()
{
}

void CompositeVolumeSprite::setInputData(vtkImageData* input)
{
    AbstractVolumeSprite::setInputData(input);
	double* spacing = input->GetSpacing();
    double min_spacing = std::min({spacing[0], spacing[1], spacing[2]});
	raycast_mapper_->SetSampleDistance(min_spacing / 2.0); 
}

void CompositeVolumeSprite::setVolumePropertyToDefault(int shift)
{
    vtkNew<vtkPiecewiseFunction> opacity_transfer_function;
    opacity_transfer_function->SetClamping(0);
    opacity_transfer_function->AddPoint(63.3 + shift, 0.0);//灰度值及不透明度值
    opacity_transfer_function->AddPoint(269.3 + shift, 0.034);
    opacity_transfer_function->AddPoint(332.2 + shift, 0.933);
    opacity_transfer_function->AddPoint(935.0 + shift, 0.99);
    opacity_transfer_function->SetClamping(1);
    vtkNew<vtkColorTransferFunction> color_transfer_function;
    color_transfer_function->SetClamping(0);
    color_transfer_function->AddRGBPoint(63.3 + shift, 0.0, 0.0, 0.0);
    color_transfer_function->AddRGBPoint(269.3 + shift, 1.0, 1.0, 0.0);
    color_transfer_function->AddRGBPoint(332.2 + shift, 1.0, 0.99920654296875, 0.9);
    color_transfer_function->AddRGBPoint(935.0 + shift,  1.0, 1.0, 1.0);
    color_transfer_function->SetClamping(1);
    vtkNew<vtkVolumeProperty> default_property;
    default_property->IndependentComponentsOn();
    default_property->SetInterpolationTypeToLinear();
    default_property->ShadeOn();
    default_property->SetScalarOpacity(opacity_transfer_function);
    default_property->SetColor(color_transfer_function);    
    volume_prop3d_->SetLODProperty(opengl_ray_id_, default_property);    
}

bool CompositeVolumeSprite::addToScene(Scene &scene)
{
    if(!AbstractVolumeSprite::addToScene(scene)){
        return false;
    }
    auto ren = scene.getRenderer();
    ren->GetCullers()->InitTraversal();
    vtkFrustumCoverageCuller::SafeDownCast(ren->GetCullers()->GetNextItem())->SetSortingStyleToBackToFront();    
    return true;
}

void CompositeVolumeSprite::makeActors(Scene &scene)
{
    addSceneProp(scene, volume_prop3d_, "volume_prop");
}

vtkProp3D* CompositeVolumeSprite::getVolume()
{
    return volume_prop3d_;
}

vtkVolumeProperty* CompositeVolumeSprite::getVolumeProperty()const
{
    vtkVolumeProperty* prop = nullptr;
    volume_prop3d_->GetLODProperty(opengl_ray_id_, &prop);
    return prop;
}

Pt3Opt CompositeVolumeSprite::pickWorldPoint(int x, int y, Scene& s)
{
    if(s.getRenderer()->HasViewProp(volume_prop3d_)){
        bool ok = vol_picker_->Pick(x, y, 0.0, s.getRenderer());
        if(ok){
            return vol_picker_->GetPickPosition();
        }
    }
    return std::nullopt;
}

NAMESPACE_END
