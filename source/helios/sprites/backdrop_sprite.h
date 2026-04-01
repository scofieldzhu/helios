/*******************************************************
* author: scofieldzhu
* time:2025/5/15
*******************************************************/
#ifndef __backdrop_sprite_h__
#define __backdrop_sprite_h__

#include "helios/sprites/helios_sprites_export.h"
#include "helios/core/sprite.h"

class vtkPolyData;

HELIOS_NAMESPACE_BEGIN

class HELIOS_SPRITES_API BackdropSprite : public Sprite
{
    SPRITE_DECL(BackdropSprite, Sprite)
public:
    BackdropSprite();
    ~BackdropSprite();

private:
    void createSource(Scene& scene) override;
	void makeActors(Scene& scene) override;
    vtkSmartPointer<vtkPolyData> polys_;
};

NAMESPACE_END

#endif