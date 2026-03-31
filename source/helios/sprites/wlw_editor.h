/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/9/2
*******************************************************/
#ifndef __change_thresold_range_editor_h__
#define __change_thresold_range_editor_h__

#include "mirfak/core/editor.h"
#include "mirfak/sprites/mirfak_sprites_export.h"
#include "mirfak/sprites/mirfak_sprites_typedef.h"

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API WLWEditor : public Editor
{
    vtkTypeMacro(WLWEditor, Editor)

public:
    static constexpr const char* WLW_UPDATE_EVENT = "ChangeThresholdRangeEditor_WLW_updated";
    struct CallData{
        Scene* event_scene = nullptr;
        double wl = 0.0;
        double ww = 0.0;
    };
    enum EditorState{
        ES_IDLE,
        ES_WORKING
    };
    void setOrthoPlanes(OrthoPlanesSprite* ortho_planes);
    void setVolume(SurfaceVolumeSprite* v);
    static WLWEditor* New();
    WLWEditor();
    ~WLWEditor();

private:
    static void SelectAction(Editor* e);
    static void EndSelectAction(Editor* e);
    static void Move3DAction(Editor* e);
    void handleSelectAction();
    void handleUnselectAction();
    void handleMove3DAction();
    SurfaceVolumeSprite* volume_ = nullptr;
    OrthoPlanesSprite* orthoplanes_ = nullptr;
    EditorState state_ = ES_IDLE;
    int last_event_x_ = -1, last_event_y_ = -1;
    CallData call_data_;
};

NAMESPACE_END

#endif