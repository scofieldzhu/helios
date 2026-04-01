/*******************************************************
* author: scofieldzhu
* time:2025/9/16
*******************************************************/
#ifndef __tube_sprite_h__
#define __tube_sprite_h__

#include "helios/core/sprite.h"
#include "helios/sprites/helios_sprites_export.h"

class vtkPolyData;
class vtkTubeFilter;
class vtkCleanPolyData;

HELIOS_NAMESPACE_BEGIN

class HELIOS_SPRITES_API TubeSprite : public Sprite
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