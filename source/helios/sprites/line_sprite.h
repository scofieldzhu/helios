/******************************************************** 
* author: scofieldzhu
* time:2025/7/18
*******************************************************/
#ifndef __line_sprite_h__
#define __line_sprite_h__

#include "helios/core/sprite.h"
#include "helios/sprites/helios_sprites_export.h"

class vtkLineSource;
class vtkProperty;

HELIOS_NAMESPACE_BEGIN

class SphereSprite;

class HELIOS_SPRITES_API LineSprite : public Sprite
{
    SPRITE_DECL(LineSprite, Sprite)
public:
    void setStartPoint(const Point3& p);
    void setEndPoint(const Point3& p);    
    LineSprite(double point_size = 1.0, double line_width = 1.0);
    ~LineSprite();

private:
    void makeActors(Scene& scene) override;
    std::unique_ptr<SphereSprite> start_point_;
    std::unique_ptr<SphereSprite> end_point_;
    vtkSmartPointer<vtkLineSource> line_source_;
    vtkSmartPointer<vtkProperty> line_prop_;
};

NAMESPACE_END

#endif