/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/7/24
*******************************************************/
#ifndef __polygon_editor_h__
#define __polygon_editor_h__

#include "helios/core/editor.h"
#include "helios/sprites/helios_sprites_export.h"

class vtkProp;

HELIOS_NAMESPACE_BEGIN

class PolygonSprite;

class HELIOS_SPRITES_API PolygonEditor : public Editor
{
    vtkTypeMacro(PolygonEditor, Editor)

public:
    enum EditorState
    {
        ES_START,
        ES_DEFINE,
        ES_MANIPULATE
    };
    bool setCurrentSprite(Sprite* s) override;
    void setCurrentVolume(vtkProp* v);
    static PolygonEditor* New();
    PolygonEditor(const PolygonEditor&) = delete;
    PolygonEditor& operator=(const PolygonEditor&) = delete;

private:
    static void SelectAction(Editor* e);
    static void EndSelectAction(Editor* e);
    static void TranslateAction(Editor* e);
    static void EndTranslateAction(Editor* e);
    static void AddPointAction(Editor* e);
    void handleSelectAction();
    void handleUnselectAction();
    void handleTranslateAction();
    void handleEndTranslateAction();
    void handleAddPointAction();
    Pt3Opt pickWorldPoint(int x, int y);
    int pickControlPointId(int x, int y);
    PolygonEditor();
    ~PolygonEditor();
    int editor_state_ = ES_START;
    PolygonSprite* current_polygon_ = nullptr;
    vtkSmartPointer<vtkProp> current_volume_;
};

NAMESPACE_END

#endif