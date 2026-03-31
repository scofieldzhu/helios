/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/10/30
*******************************************************/
#ifndef __torus_sprite_h__
#define __torus_sprite_h__

#include "mirfak/core/sprite.h"
#include "mirfak/sprites/mirfak_sprites_export.h"

class vtkDiskSource;
class vtkLinearExtrusionFilter;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API TorusSprite : public Sprite
{
	SPRITE_DECL(TorusSprite, Sprite)
public:
	void setCenter(const Point3& pt);
	Point3 getCenter()const;
	void setNormal(const Vec3& n);
	Vec3 getNormal()const;
	TorusSprite(double inner_radius, double outer_radius, double thickness);
	~TorusSprite();

private:
	void createSource(Scene& scene) override;
	void makeActors(Scene& scene) override;
	vtkSmartPointer<vtkDiskSource> torus_source_;
	vtkSmartPointer<vtkLinearExtrusionFilter> extruder_;
};

NAMESPACE_END


#endif