/******************************************************** 
* author: scofieldzhu
* time:2025/4/8
*******************************************************/
#ifndef __sprite_cell_picker_h__
#define __sprite_cell_picker_h__

#include "helios/core/sprite_picker.h"
#include "helios/core/helios_core_export.h"

HELIOS_NAMESPACE_BEGIN

class HELIOS_CORE_API SpriteCellPicker : public SpritePicker
{
public:
    void setExcludedSprites(const ConstSpriteList& sprites) override{
        excluded_sprites_ = sprites;
    }
    void setPickTarget(const Sprite* target) override{
        target_sprite_ = target;
    }
    PickInfoList pick(Scene& scene, int x, int y) override;    
    SpriteCellPicker(const Sprite* target = nullptr);
    ~SpriteCellPicker() = default;

private:
    ConstSpriteList excluded_sprites_;
    const Sprite* target_sprite_;
};

NAMESPACE_END

#endif