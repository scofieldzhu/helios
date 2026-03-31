/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/9/16
*******************************************************/
#ifndef __tube_sprite_h__
#define __tube_sprite_h__

#include "mirfak/core/sprite.h"
#include "mirfak/sprites/mirfak_sprites_export.h"

class vtkPolyData;
class vtkTubeFilter;
class vtkCleanPolyData;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API TubeSprite : public Sprite
{
    SPRITE_DECL(TubeSprite, Sprite)
public:
    Pt3List getPoints()const;
    void setPoints(const Pt3List& pts);
    void setSourcePolys(vtkPolyData* ply);
    TubeSprite(double r);
    ~TubeSprite();

private:
	void createSource(Scene& scene) override;
	void makeActors(Scene& scene) override;
    vtkSmartPointer<vtkTubeFilter> tube_source_;
    vtkSmartPointer<vtkCleanPolyData> clean_polydata_;
};

NAMESPACE_END

#endif