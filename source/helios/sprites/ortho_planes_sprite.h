/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/5/15
*******************************************************/
#ifndef __ortho_planes_sprite_h__
#define __ortho_planes_sprite_h__

#include "mirfak/core/sprite.h"
#include "mirfak/sprites/mirfak_sprites_export.h"

class vtkImageData;
class vtkLookupTable;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API OrthoPlanesSprite : public Sprite
{      
	SPRITE_DECL(OrthoPlanesSprite, Sprite)
public:    
    using Sprite::setOpacity;
    Signal<Point3> center_changed;
    Signal<std::size_t> input_added;
    Signal<std::size_t> input_removed;
    Signal<std::size_t> input_updated;
    Signal<std::size_t> opacity_updated;
    Signal<std::size_t> lookuptable_updated;
    void reset();
    void setSliceInterpolate(bool interpolate);
    bool sliceInterpolate();
    void setTextureInterpolate(bool interpolate);
    bool textureInterpolate();
    void setOrthoCenter(const Point3& center);	    
    Point3 getOrthoCenter() const;	
    Point3 calcOrthoCenter()const;
    std::size_t addInputData(vtkImageData* input);
    void removeInputData(std::size_t id = 0);    
    void setInputData(vtkImageData* input, std::size_t id = 0);
    void setOpacity(const Scene& scene, std::size_t id, double alpha);
    void setOpacity(std::size_t id, double alpha);
    double getOpacity(std::size_t id) const;
    vtkImageData* getInputData(std::size_t id = 0) const;
    std::size_t getNumberOfInputs() const;
    void setLookupTable(vtkLookupTable* table, std::size_t id = 0);
    vtkLookupTable* getLookupTable(std::size_t id)const;
    void addTextureOffPane(Scene* scene);
    std::array<SlicePlaneSprite*, 3> getPlanes() const;
	SlicePlaneSprite* getAxialPlane() const;
	SlicePlaneSprite* getCoronalPlane() const;
	SlicePlaneSprite* getSagittalPlane() const;
    OrthoPlanesSprite();
    ~OrthoPlanesSprite();

private:       
    void initPlaneOrientation();
    std::array<std::unique_ptr<SlicePlaneSprite>, 3> planes_;
};

NAMESPACE_END

#endif
