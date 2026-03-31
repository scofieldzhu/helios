/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/5/15
*******************************************************/
#ifndef __backdrop_sprite_h__
#define __backdrop_sprite_h__

#include "mirfak/sprites/mirfak_sprites_export.h"
#include "mirfak/core/sprite.h"

class vtkPolyData;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API BackdropSprite : public Sprite
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