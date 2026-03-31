/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/10/31
*******************************************************/
#ifndef __arrow_sprite_h__
#define __arrow_sprite_h__

#include "mirfak/core/sprite.h"
#include "mirfak/sprites/mirfak_sprites_export.h"

class vtkArrowSource;
class vtkTransformPolyDataFilter;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API ArrowSprite : public Sprite
{
	SPRITE_DECL(ArrowSprite, Sprite)
public:
	void setLength(double len);
	double length()const{ return length_; }
	void setStartPoint(const Point3& pt);
	const Point3& startPoint()const{ return start_point_; }
	void setDirection(const Vec3& v);
	const Point3& direction()const{ return direction_; }
	ArrowSprite(double shaft_radius, double tip_radius, double tip_len);
	~ArrowSprite();

private:
	void transformData();
	void createSource(Scene& scene) override;
	void makeActors(Scene& scene) override;
	vtkSmartPointer<vtkArrowSource> arrow_source_;
	vtkSmartPointer<vtkTransformPolyDataFilter> trans_poly_filter_;
	double length_ = 12.0;
	Point3 start_point_ = {0.0, 0.0, 0.0};
	Vec3 direction_ = {1.0, 0.0, 0.0};
};

NAMESPACE_END

#endif