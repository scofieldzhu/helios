/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/6/24
*******************************************************/
#ifndef __plane_intersection_line_sprite_h__
#define __plane_intersection_line_sprite_h__

#include "mirfak/sprites/plane_cutter_sprite.h"

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API PlaneIntersectionLineSprite : public Sprite
{
    SPRITE_DECL(PlaneIntersectionLineSprite, Sprite)
public:
    enum InteractionState
    {
        IS_OUTSIDE = 0,
        IS_ROTATING,
        IS_TRANSLATE_H,
        IS_TRANSLATE_V,
        IS_MOVE_CENTER
    };
    struct InteractionStateData
    {
        Point2i event_dpos;
        double angle = 0.0;
        SlicePlaneSprite* on_plane = nullptr;
        SlicePlaneSprite* picked_plane = nullptr;
        SlicePlaneSprite* cross_plane = nullptr;
        Scene* scene = nullptr;
    };
    auto cutPlane(){ return cut_plane_; }
    auto targetPlane(){ return target_plane_; }
    auto crossPlane(){ return cross_plane_; }
    void setLineColor(const Color& hline_clr, const Color& vline_clr);
    void setPlanes(SlicePlaneSprite& cut_plane, SlicePlaneSprite& target_plane, SlicePlaneSprite& cross_plane);
    int computeEditState(Scene& s, int x, int y, int modify = 0) override;
    const void* getEditStateData()const override;
    PlaneIntersectionLineSprite();
    ~PlaneIntersectionLineSprite();

private:
    SlicePlaneSprite* cut_plane_ = nullptr;
    SlicePlaneSprite* target_plane_ = nullptr;
    SlicePlaneSprite* cross_plane_ = nullptr;
    std::unique_ptr<PlaneCutterSprite> intersection_line_;
    std::unique_ptr<PlaneCutterSprite> cross_intersection_line_;
    InteractionStateData state_data_;
};

NAMESPACE_END

#endif