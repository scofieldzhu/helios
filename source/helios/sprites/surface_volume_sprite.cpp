/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/8/25
*******************************************************/
#include "surface_volume_sprite.h"
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkImageData.h>
#include <vtkTransform.h>
#include <vtkVolumeProperty.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkLODProp3D.h>
#include <vtkPlaneCollection.h>
#include <vtkContourValues.h>
#include <vtkNamedColors.h>
#include <vtkColorTransferFunction.h>
#include <vtkPlane.h>
//#include <vtkMeshVolumeScalarPicker.h>
#include <vtkVolume.h>
#include "mirfak/core/scene.h"
#include "mirfak/basic/log_service.h"

MIRFAK_NAMESPACE_BEGIN

namespace{
    constexpr double ISO1 = 0.0;
    constexpr double ISO2 = 5000.0;
}

SurfaceVolumeSprite::SurfaceVolumeSprite(vtkIdType max_memory_bytes)
    :AbstractVolumeSprite(max_memory_bytes),
    volume_(vtkSmartPointer<vtkVolume>::New())
{
    raycast_mapper_->AutoAdjustSampleDistancesOff();
    raycast_mapper_->SetSampleDistance(0.3);
    raycast_mapper_->SetBlendModeToIsoSurface();
    setVolumePropertyToDefault();
    volume_->SetMapper(raycast_mapper_);
}

SurfaceVolumeSprite::~SurfaceVolumeSprite()
{
}

void SurfaceVolumeSprite::setVolumePropertyToDefault()
{
    vtkNew<vtkColorTransferFunction> clr_trans_func;
    clr_trans_func->AddRGBPoint(ISO1, 1, 0.87, 0.55);
    clr_trans_func->AddRGBPoint(ISO2, 1, 0.87, 0.55);
    vtkNew<vtkPiecewiseFunction> scalar_opacity_func;
    scalar_opacity_func->AddPoint(ISO1, 1.0);
    scalar_opacity_func->AddPoint(ISO2, 1.0);
    vtkNew<vtkVolumeProperty> vol_prop;
    vol_prop->ShadeOn();
    vol_prop->SetInterpolationTypeToLinear();
    vol_prop->SetColor(clr_trans_func);
    vol_prop->SetScalarOpacity(scalar_opacity_func);
    vol_prop->SetAmbient(0.1);
    vol_prop->SetSpecular(0.2);
    vol_prop->SetDiffuse(0.7);
    volume_->SetProperty(vol_prop);
    vol_prop->GetIsoSurfaceValues()->SetValue(0, ISO1);
    vol_prop->GetIsoSurfaceValues()->SetValue(1, ISO2);
}

vtkProp3D* SurfaceVolumeSprite::getVolume()
{
    return volume_;
}

vtkVolumeProperty* SurfaceVolumeSprite::getVolumeProperty() const
{
    return volume_->GetProperty();
}

void SurfaceVolumeSprite::setVolumeThresholdValue(int contours_number, double start, double end)
{
    auto contour_values = getVolumeProperty()->GetIsoSurfaceValues();
    contour_values->GenerateValues(contours_number, start, end);
    getVolumeProperty()->Modified();
    raycast_mapper_->Modified();
    modified();
}

std::array<double, 2> SurfaceVolumeSprite::getThresholdRange() const
{
    auto values = getVolumeProperty()->GetIsoSurfaceValues();
    if(values->GetNumberOfContours() < 2){
        return {0, 0};
    }
    auto start = values->GetValue(0);
    auto end = values->GetValue(values->GetNumberOfContours() - 1);
    return std::array<double, 2>({start, end});
}

void SurfaceVolumeSprite::setOpacity(double opacity)
{
    auto vol_prop = getVolumeProperty();
    auto scalar_op_func = vol_prop->GetScalarOpacity(0);        
    auto cont_values = vol_prop->GetIsoSurfaceValues();
    scalar_op_func->RemoveAllPoints();
    for(auto i = 0; i < cont_values->GetNumberOfContours(); ++i){
        scalar_op_func->AddPoint(cont_values->GetValue(i), opacity);
    }
    scalar_op_func->Modified();    
    vol_prop->Modified();
    modified();
}

void SurfaceVolumeSprite::makeActors(Scene &scene)
{
    addSceneProp(scene, volume_, "volume_prop");
}

Pt3Opt SurfaceVolumeSprite::pickWorldPoint(int x, int y, Scene &s)
{
    if(!checkConnective(s)){
        return std::nullopt;
    }
    // vtkNew<vtkMeshVolumeScalarPicker> mvsp;
    // double p[3];
    // if(!mvsp->Pick((double)x, (double)y, 0.0, s.getRenderer(), getInput(), getThresholdRange()[0], p)){
    //     return std::nullopt;
    // }
    // return Point3(p);
    return std::nullopt;
}

NAMESPACE_END
