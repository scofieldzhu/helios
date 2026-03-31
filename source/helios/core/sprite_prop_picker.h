/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/4/18
*******************************************************/
#ifndef __sprite_prop_picker_h__
#define __sprite_prop_picker_h__

#include <vtkSmartPointer.h>
#include "mirfak/core/sprite_picker.h"
#include "mirfak/core/mirfak_core_export.h"

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_CORE_API SpritePropPicker : public SpritePicker
{
public:
    void setExcludedSprites(const ConstSpriteList& sprites) override{ excluded_sprites_ = sprites; }
    void setPickTarget(const Sprite* target) override{ target_ = target; }
    PickInfoList pick(Scene& scene, int x, int y) override;
    SpritePropPicker() = default;
    ~SpritePropPicker() = default;

private:
    const Sprite* target_ = nullptr;
    ConstSpriteList excluded_sprites_;
};

NAMESPACE_END

#endif