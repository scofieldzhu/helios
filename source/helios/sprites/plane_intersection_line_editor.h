/******************************************************** 
* author: scofieldzhu
* time:2025/6/26
*******************************************************/
#ifndef __plane_intersection_line_editor_h__
#define __plane_intersection_line_editor_h__

#include "helios/core/editor.h"
#include "helios/sprites/plane_intersection_line_sprite.h"

HELIOS_NAMESPACE_BEGIN

class HELIOS_SPRITES_API PlaneIntersectionLineEditor : public Editor
{
    vtkTypeMacro(PlaneIntersectionLineEditor, Editor)
public:
    static constexpr const char* ROTATING_EVENT = "PlaneIntersectionLineEditor_Rotating";
    static constexpr const char* TRANSLATING_EVENT = "PlaneIntersectionLineEditor_Translating";
    enum EditorState{
        ES_START,
        ES_ROTATING,
        ES_TRANSLATING_H,
        ES_TRANSLATING_V,
        ES_MOVING_CENTER
    };
    static PlaneIntersectionLineEditor* New();
    bool setCurrentSprite(Sprite* s) override;
    
private:
    void updateCursorStyle(int state);
    static void SelectAction(Editor* e);
    void handleSelectAction();
    static void EndSelectAction(Editor* e);
    void handleEndSelectAction();
    static void MoveAction(Editor* e);
    void handleMoveAction();
    void onRotateStarted(const Point2i& pos);
    void onRotating(const Point2i& pos);
    void onRotateEnd(const Point2i& pos);
    void onTranslateHStarted(const Point2i& pos);
    void onTranslatingH(const Point2i& pos);
    void onTranslateHEnd(const Point2i& pos);
    void onTranslateVStarted(const Point2i& pos);
    void onTranslatingV(const Point2i& pos);
    void onTranslateVEnd(const Point2i& pos);
    void onMoveCenterStarted(const Point2i& pos);
    void onMovingCenter(const Point2i& pos);
    void onMoveCenterEnd(const Point2i& pos);
    double calcRotateAngleOnPlane(const Point3& now_wpos, const Point3& last_wpos, const Point3& center, const Point3& normal) const;
    Point3 calcCurrentOrthoCenter()const;
    bool movePlane(SlicePlaneSprite& plane, const Point3& drag_wpt);
    PlaneIntersectionLineEditor();
    ~PlaneIntersectionLineEditor();
    int editor_state_ = ES_START;
    Point3 last_drag_wpos_;
    PlaneIntersectionLineSprite::InteractionStateData drag_state_info_;
    PlaneIntersectionLineSprite* pil_ = nullptr;
};

NAMESPACE_END

#endif
