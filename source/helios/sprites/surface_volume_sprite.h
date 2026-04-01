/*******************************************************
* author: scofieldzhu
* time:2025/8/25
*******************************************************/
#ifndef __surface_volume_sprite_h__
#define __surface_volume_sprite_h__

#include "helios/sprites/abstract_volume_sprite.h"

class vtkVolume;

HELIOS_NAMESPACE_BEGIN

class HELIOS_SPRITES_API SurfaceVolumeSprite : public AbstractVolumeSprite
{
    SPRITE_DECL(SurfaceVolumeSprite, AbstractVolumeSprite)
public:
    vtkProp3D* getVolume() override;
    vtkVolumeProperty* getVolumeProperty()const override;
    void setVolumeThresholdValue(int contours_number, double start, double end);
    std::array<double, 2> getThresholdRange()const;
    void setOpacity(double opacity) override;
    Pt3Opt pickWorldPoint(int x, int y, Scene& s) override;
    SurfaceVolumeSprite(vtkIdType max_memory_bytes);
    ~SurfaceVolumeSprite();

private:
    void makeActors(Scene& scene) override;
    void setVolumePropertyToDefault();  
    vtkSmartPointer<vtkVolume> volume_;
};

NAMESPACE_END

#endif