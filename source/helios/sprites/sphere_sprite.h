/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/7/10
*******************************************************/
#ifndef __sphere_sprite_h__
#define __sphere_sprite_h__

#include "helios/core/sprite.h"
#include "helios/sprites/helios_sprites_export.h"

class vtkSphereSource;

HELIOS_NAMESPACE_BEGIN

class HELIOS_SPRITES_API SphereSprite : public Sprite
{
    SPRITE_DECL(SphereSprite, Sprite)
public:
    void setCenter(const Point3& pt);
    Point3 getCenter()const;
    void setRadius(double r);
    double getRadius()const;
    SphereSprite();
    ~SphereSprite();

private:
	void createSource(Scene& scene) override;
	void makeActors(Scene& scene) override;
    vtkSmartPointer<vtkSphereSource> sphere_source_;
};

NAMESPACE_END

#endif