/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/5/15
*******************************************************/
#ifndef __plane_sprite_h__
#define __plane_sprite_h__

#include "mirfak/core/sprite.h"
#include "mirfak/sprites/mirfak_sprites_export.h"

class vtkPlaneSource;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API PlaneSprite : public Sprite
{
    SPRITE_DECL(PlaneSprite, Sprite)
public:
    void setPoints(const Point3& origin, const Point3& pt1, const Point3& pt2);
    PlaneSprite();
    ~PlaneSprite();

private:
    void makeActors(Scene& scene) override;
    vtkSmartPointer<vtkPlaneSource> plane_source_;
};

NAMESPACE_END

#endif