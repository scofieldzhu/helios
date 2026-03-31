/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/4/8
*******************************************************/
#ifndef __sprite_picker_h__
#define __sprite_picker_h__

#include "mirfak/core/mirfak_core_typedef.h"
#include "mirfak/core/pick_info.h"

MIRFAK_NAMESPACE_BEGIN

class SpritePicker
{
public:
    virtual void setExcludedSprites(const ConstSpriteList& sprites) = 0;
    virtual void setPickTarget(const Sprite* target) = 0;
    virtual PickInfoList pick(Scene& scene, int x, int y) = 0;    
    virtual ~SpritePicker() = default;
};

NAMESPACE_END

#endif
