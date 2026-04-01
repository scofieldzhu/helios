/*******************************************************
* author: scofieldzhu
* time:2025/9/10
*******************************************************/
#ifndef __surgical_path_editor_h__
#define __surgical_path_editor_h__

#include "helios/core/editor.h"
#include "helios/sprites/surgical_path_sprite.h"

HELIOS_NAMESPACE_BEGIN

class SphereSprite;
class AbstractVolumeSprite;

class HELIOS_SPRITES_API SurgicalPathEditor : public Editor
{
    vtkTypeMacro(SurgicalPathEditor, Editor)
public:
    Signal<std::string> PathCreatedSignal;
    enum EditorState
    {
        ES_START,
        ES_DEFINE,
        ES_MANIPULATE
    };
    bool setCurrentSprite(Sprite* s) override;
    void setCurrentVolume(AbstractVolumeSprite* v);
    void setSurgicalPathGroup(SurgicalPathGroup* group){ path_group_ = group; }
    enum EditorAction{
        EA_NONE,
        EA_CREATE_PATH,
        EA_EDIT_PATH
    };
    void setEditorAction(EditorAction e){ ea_ = e; }
    auto editorAction()const{ return ea_; }
    static SurgicalPathEditor* New();
    SurgicalPathEditor(const SurgicalPathEditor&) = delete;
    SurgicalPathEditor& operator=(const SurgicalPathEditor&) = delete;

private:
    static void SelectAction(Editor* e);
    static void EndSelectAction(Editor* e);
    static void MovePointAction(Editor* e);
    void handleSelectAction();
    void handleUnselectAction();
    void handleTranslateAction();
    void handleEndTranslateAction();
    void handleMovePointAction();
    void updateCursorStyle(int state);
    Pt3Opt pickWorldPoint(int x, int y);
    SurgicalPathEditor();
    ~SurgicalPathEditor();
    int editor_state_ = ES_START;
    SurgicalPathSprite* current_path_ = nullptr;
    AbstractVolumeSprite* current_volume_ = nullptr;
    std::shared_ptr<SphereSprite> end_point_sphere_, start_point_sphere_;
    SurgicalPathSprite* current_created_path_ = nullptr;
    SurgicalPathGroup* path_group_;    
    bool left_mouse_key_down_ = false;
    Pt3Opt last_move_point_;
    EditorAction ea_ = EA_NONE;
};

NAMESPACE_END

#endif