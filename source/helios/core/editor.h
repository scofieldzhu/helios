/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/7/1
*******************************************************/
#ifndef __editor_h__
#define __editor_h__

#include <vtkRenderWindowInteractor.h>
#include <vtkCallbackCommand.h>
#include "helios/core/scene.h"
#include "helios/core/action_slot_mapper.h"
#include "helios/core/action_event.h"

HELIOS_NAMESPACE_BEGIN

class EventTranslator;

class HELIOS_CORE_API Editor : public vtkObject
{
    vtkTypeMacro(Editor, vtkObject);
public:
    virtual void setEnabled(bool e);
    bool enabled()const{ return enabled_; }
    Scene* currentScene(){ return current_scene_; }
    Scene* focusScene(){ return focus_scene_; }
    virtual void setCurrentInteractor(vtkRenderWindowInteractor* i);
    auto currentInteractor(){ return current_interactor_; }
    void setEventPriority(double p);
    double eventPriority()const;
    virtual bool setCurrentSprite(Sprite* s);
    Sprite* currentSprite(){ return current_sprite_; }
    void setAllowedScenes(const SceneList& scenes);    
    const auto& allowedScenes()const{ return allowed_scenes_; }
    void setDisplayScenes(const SceneList& scenes);
    const auto& displayScenes()const{ return display_scenes_; }
    bool isAllowedScene(Scene* s)const;
    void continuePassEventToNext(bool s = true);

protected:
    static void ProcessEventHandler(vtkObject*, unsigned long, void*, void*);
    virtual bool handleEventHandler(vtkObject* object, unsigned long vtk_event, void* client_data, void* call_data);
    Scene* findPockedScene(int x, int y)const;
    Editor();
    ~Editor();
    EventTranslator* event_translator_;
    ActionSlotMapper action_slot_mapper_;
    void* event_call_data_ = nullptr;
    Sprite* current_sprite_ = nullptr;
    vtkSmartPointer<vtkCallbackCommand> event_callback_cmd_;    
    bool enabled_ = false;    
    vtkRenderWindowInteractor* current_interactor_ = nullptr;
    Scene* current_scene_ = nullptr;
    Scene* focus_scene_ = nullptr;
    SceneList allowed_scenes_;
    SceneList display_scenes_;
    double event_priority_ = 0.5;        
};

NAMESPACE_END

#endif