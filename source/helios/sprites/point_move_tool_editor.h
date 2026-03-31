/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/10/31
*******************************************************/
#ifndef __point_move_tool_editor_h__
#define __point_move_tool_editor_h__

#include "mirfak/core/editor.h"
#include "mirfak/sprites/point_move_tool_sprite.h"

MIRFAK_NAMESPACE_BEGIN

class AbstractVolumeSprite;
class SphereSprite;

class MIRFAK_SPRITES_API PointMoveToolEditor : public Editor
{
	vtkTypeMacro(PointMoveToolEditor, Editor)
public:
    Signal<Point3> PointMovedSignal;
	static PointMoveToolEditor* New();
    bool setCurrentSprite(Sprite* s) override;
    enum EditorState{
        ES_NONE,
        ES_START,
        ES_TRANSLATING,
        ES_ROTATING
    };
    void setEnabled(bool e) override;
	PointMoveToolEditor(const PointMoveToolEditor&) = delete;
	PointMoveToolEditor& operator=(const PointMoveToolEditor&) = delete;

private:
    static void SelectAction(Editor* e);
    static void EndSelectAction(Editor* e);
    static void MoveAction(Editor* e);
    void handleSelectAction();
    void handleUnselectAction();
    void handleTranslateAction();
    void handleEndTranslateAction();
    void handleMoveAction();
    void doTranslate(const Point3& picked_pos);
    void doRotate(const Point3& picked_pos);
    void updateCursorStyle(int state);
    Pt3Opt pickWorldPoint(int x, int y);
    PointMoveToolEditor();
    ~PointMoveToolEditor();
    PointMoveToolSprite* move_tool_ = nullptr;
    EditorState editor_state_ = ES_NONE;
    bool left_button_down_ = false;
    PointMoveToolSprite::EditStateData start_drag_data_;
    Pt3Opt last_drag_point_;
};

NAMESPACE_END

#endif