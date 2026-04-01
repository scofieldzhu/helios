/*******************************************************
* author: scofieldzhu
* time:2025/5/15
*******************************************************/
#include "ortho_planes_sprite.h"
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include "helios/basic/plane.h"
#include "helios/basic/line.h"
#include "helios/core/slice_plane_sprite.h"
#include "helios/core/scene.h"

HELIOS_NAMESPACE_BEGIN

OrthoPlanesSprite::OrthoPlanesSprite()  
{        
    for(auto i = 0; i < 3; ++i){
        planes_[i] = std::make_unique<SlicePlaneSprite>();    
        if(i == 0){
            planes_[i]->setName("AxialPlane");
        }else if(i == 1){
            planes_[i]->setName("CoronalPlane");
        }else if(i == 2){
            planes_[i]->setName("SagittalPlane");
        }
        planes_[i]->setRestrictToVolume(true);   
        addChild(*planes_[i]);  
        planes_[i]->setRestrictToVolume(false);
    }
}

OrthoPlanesSprite::~OrthoPlanesSprite()
{
    disconnectScenes(nullptr);     
}

Point3 OrthoPlanesSprite::getOrthoCenter() const
{
    auto axial_plane = getAxialPlane();
    auto coronal_plane = getCoronalPlane();
    auto sagittal_plane = getSagittalPlane();
    if(axial_plane == nullptr || coronal_plane == nullptr || sagittal_plane == nullptr){
        return {0.0, 0.0, 0.0};
    }
    //find the intersection point of the planes:
    //this is done by solving the three plane equations
    Vec3 uxyz = axial_plane->getNormal();
    Vec3 vxyz = coronal_plane->getNormal();
    Vec3 wxyz = sagittal_plane->getNormal();
    Point3 axyz = axial_plane->getOrigin();
    Point3 bxyz = coronal_plane->getOrigin();
    Point3 cxyz = sagittal_plane->getOrigin();
    double a = uxyz.dot(axyz);
    double b = vxyz.dot(bxyz);
    double c = wxyz.dot(cxyz);
    double elements[] = { 
        uxyz[0], uxyz[1], uxyz[2], 0,
        vxyz[0], vxyz[1], vxyz[2], 0,
        wxyz[0], wxyz[0], wxyz[0], 0,
        0,       0,       0,       1
    };
    vtkNew<vtkMatrix4x4> rotate_matrix;
    rotate_matrix->DeepCopy(elements);
    rotate_matrix->Invert(); //convert local to world 
    double pt_array[] = {a, b, c, 0};
    return rotate_matrix->MultiplyDoublePoint(pt_array);
}

void OrthoPlanesSprite::setOrthoCenter(const Point3& center)
{    
    for(auto idx = 0; idx < 3; ++idx){
        Point3 start_pt = planes_[idx]->startPoint();
        Vec3 normal = planes_[idx]->getNormal();        
        planes_[idx]->setPosition(Plane(start_pt, normal).getProjectDistance(center));
    }
    modified();
    center_changed.invoke(center);
}

Point3 OrthoPlanesSprite::calcOrthoCenter() const
{    
    Plane plane0(planes_[0]->getOrigin(), planes_[0]->getNormal());    
    Plane plane1(planes_[1]->getOrigin(), planes_[1]->getNormal());
    Plane plane2(planes_[2]->getOrigin(), planes_[2]->getNormal());
    return Plane::CalcCenterOfTriplePlanes(plane0, plane1, plane2).value();
}

std::size_t OrthoPlanesSprite::addInputData(vtkImageData* input)
{    
    for(auto idx = 0; idx < 3; ++idx){
        planes_[idx]->addInputData(input);
    }
    std::size_t input_num = planes_[0]->getNumberOfInputs();
    if(input_num == 1){
        initPlaneOrientation();
    }
    input_added.invoke(input_num - 1);
    return input_num;
}

void OrthoPlanesSprite::removeInputData(std::size_t id)
{
    for(auto idx = 0; idx < 3; ++idx){
        planes_[idx]->removeInputData(id);
    }
    input_removed.invoke(id);
}

void OrthoPlanesSprite::setOpacity(std::size_t index, double alpha)
{
    for(auto idx = 0; idx < 3; ++idx){
        planes_[idx]->setOpacity(nullptr, alpha, index);
    }        
    opacity_updated.invoke(index);
}

void OrthoPlanesSprite::setOpacity(const Scene& scene, std::size_t index, double alpha)
{
    for(auto idx = 0; idx < 3; ++idx){
        planes_[idx]->setOpacity(&scene, alpha, index);
    }
}

double OrthoPlanesSprite::getOpacity(std::size_t id) const
{
    return planes_[0]->getOpacity(nullptr, id);
}

void OrthoPlanesSprite::reset()
{
    disconnectScenes(nullptr);
    for(auto idx = 0; idx < 3; ++idx){
        planes_[idx]->reset();
    }
}

void OrthoPlanesSprite::setInputData(vtkImageData* input, std::size_t target_idx)
{
    for(auto idx = 0; idx < 3; ++idx){
        planes_[idx]->setInputData(input, target_idx);
    }
    if(target_idx == 0){
        initPlaneOrientation();
    }
    input_updated.invoke(target_idx);
}

vtkImageData* OrthoPlanesSprite::getInputData(std::size_t idx) const
{
    return planes_[0]->getInputData(idx);    
}

std::size_t OrthoPlanesSprite::getNumberOfInputs() const
{
    return planes_[0]->getNumberOfInputs();    
}

void OrthoPlanesSprite::setLookupTable(vtkLookupTable* table, std::size_t id /*= 0*/)
{
    for(auto idx = 0; idx < 3; ++idx){
        planes_[idx]->setLookupTable(table, id);
    }          
    lookuptable_updated.invoke(id);
}

vtkLookupTable* OrthoPlanesSprite::getLookupTable(std::size_t id) const
{
    return id < 3 ? planes_[id]->getLookupTable(id) : nullptr;
}

void OrthoPlanesSprite::addTextureOffPane(Scene* s)
{
    for(auto idx = 0; idx < 3; ++idx){
        planes_[idx]->addTextureOffScene(s);
    }
}

std::array<SlicePlaneSprite*, 3> OrthoPlanesSprite::getPlanes() const
{
    return std::to_array({planes_[0].get(), planes_[1].get(), planes_[2].get()});
}

SlicePlaneSprite* OrthoPlanesSprite::getAxialPlane() const
{
    return planes_[0].get();
}

SlicePlaneSprite* OrthoPlanesSprite::getCoronalPlane() const
{
    return planes_[1].get();
}

SlicePlaneSprite* OrthoPlanesSprite::getSagittalPlane() const
{
    return planes_[2].get();
}

void OrthoPlanesSprite::initPlaneOrientation()
{    
    planes_[0]->setPlaneType(SlicePlaneSprite::kAxial);
    planes_[1]->setPlaneType(SlicePlaneSprite::kCoronal);
    planes_[2]->setPlaneType(SlicePlaneSprite::kSagittal);
}

void OrthoPlanesSprite::setSliceInterpolate(bool interpolate)
{
    for(auto idx = 0; idx < 3; ++idx){
        planes_[idx]->setResliceInterpolate(interpolate);
    }
    modified();
}

bool OrthoPlanesSprite::sliceInterpolate()
{    
    return planes_[0]->resliceInterpolate();
}

void OrthoPlanesSprite::setTextureInterpolate(bool interpolate)
{
    for(auto idx = 0; idx < 3; ++idx){
        planes_[idx]->setTextureInterpolate(interpolate);
    }
    modified();
}

bool OrthoPlanesSprite::textureInterpolate()
{
    return planes_[0]->textureInterpolate();
}

NAMESPACE_END
