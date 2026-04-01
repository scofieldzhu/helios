/******************************************************** 
* author: scofieldzhu
* time:2025/4/8
*******************************************************/
#ifndef __sprite_picker_h__
#define __sprite_picker_h__

#include "helios/core/helios_core_typedef.h"
#include "helios/core/pick_info.h"

HELIOS_NAMESPACE_BEGIN

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
