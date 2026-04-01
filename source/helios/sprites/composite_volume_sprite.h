/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/5/15
*******************************************************/

/**********************************************************
CompositeVolumeSprite - volume rendering using color transfer function

The VolumeSprite uses VTK's built-in volume rendering functionality.
Texture-accelerated volume rendering is used while the user is interacting
with the volume, and ray-cast volume rendering is used for high-quality
volume rendering.

Interaction with the planes (i.e. slicing into the planes) is done
via the ClippingCubeBubble.
***********************************************************/
#ifndef __composite_volume_sprite_h__
#define __composite_volume_sprite_h__

#include "helios/sprites/abstract_volume_sprite.h"

class vtkLODProp3D;
class vtkVolumePicker;

HELIOS_NAMESPACE_BEGIN

class HELIOS_SPRITES_API CompositeVolumeSprite: public AbstractVolumeSprite
{  
	SPRITE_DECL(CompositeVolumeSprite, AbstractVolumeSprite)
public:        
    void setInputData(vtkImageData* input) override;
    vtkProp3D* getVolume() override;
    vtkVolumeProperty* getVolumeProperty()const override;   
    Pt3Opt pickWorldPoint(int x, int y, Scene& s) override;
	CompositeVolumeSprite(vtkIdType max_mem_bytes);
    ~CompositeVolumeSprite();

private:
    bool addToScene(Scene& scene) override;
    void makeActors(Scene& scene) override;
    void setVolumePropertyToDefault(int shift);   
    int opengl_ray_id_ = 0;
    vtkSmartPointer<vtkLODProp3D> volume_prop3d_;
    vtkSmartPointer<vtkVolumePicker> vol_picker_;
};

NAMESPACE_END

#endif
