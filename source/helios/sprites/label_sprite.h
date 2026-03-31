/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/9/3
*******************************************************/
#ifndef __label_sprite_h__
#define __label_sprite_h__

#include "mirfak/sprites/mirfak_sprites_export.h"
#include "mirfak/core/sprite.h"

class vtkTextActor;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API LabelSprite : public Sprite
{
    SPRITE_DECL(LabelSprite, Sprite)
public:
    void setText(const char* text);
    void setFontFamily(int f);
    void setFontFile(const char* font_file);
    void setBold(bool b);
    void setFontSize(int size);
    void setColor(const Color& color) override;
    void setPosition(double x, double y);
    void setOpacity(double opacity) override;
    void setJustification(int j);
    LabelSprite();
    ~LabelSprite();

private:
    void initTextProps();
    bool addToScene(Scene& scene) override;
    void makeActors(Scene& scene) override;
    vtkSmartPointer<vtkTextActor> text_actor_;
};

NAMESPACE_END

#endif