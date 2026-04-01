/******************************************************** 
* author: scofieldzhu
* time:2025/7/1
*******************************************************/
#include "editor.h"
#include <vtkEvent.h>
#include <vtkEventData.h>
#include <vtkRenderer.h>
#include "event_translator.h"
#include "helios/basic/log_service.h"
#include "sprite.h"
#include "scene.h"
#include "render_widget.h"

HELIOS_NAMESPACE_BEGIN

Editor::Editor()
    :event_translator_(new EventTranslator()),
    event_callback_cmd_(vtkSmartPointer<vtkCallbackCommand>::New())
{
    event_callback_cmd_->SetCallback(Editor::ProcessEventHandler);
    event_callback_cmd_->SetClientData(this);
    action_slot_mapper_.setEventTranslator(event_translator_);
}

Editor::~Editor()
{
    setEnabled(false);
    delete event_translator_;    
}

bool Editor::setCurrentSprite(Sprite* s)
{
    current_sprite_ = s;
    return true;
}

void Editor::setAllowedScenes(const SceneList& scenes)
{
    allowed_scenes_ = scenes;
}

bool Editor::isAllowedScene(Scene* s) const
{
    return s == nullptr ? false : std::find(allowed_scenes_.begin(), allowed_scenes_.end(), s) != allowed_scenes_.end();
}

void Editor::continuePassEventToNext(bool s)
{
    event_callback_cmd_->SetAbortFlag(!s);
}

void Editor::setCurrentInteractor(vtkRenderWindowInteractor* i)
{
    current_interactor_ = i;
}

void Editor::setEventPriority(double p)
{
    event_priority_ = p;
}

void Editor::setEnabled(bool enabled)
{
    if(enabled){
        SPDLOG_TRACE("Enabling sprite editor({})", GetClassName());
        if(enabled_){ //already enabled, just return!
            return;
        }
        // cannot do anything if current interactor not set yet!
        if(current_interactor_ == nullptr){
            SPDLOG_ERROR("The interactor must be set prior to enabling the editor!");
            return;
        }
        // make sure current sprite's existence!
        // if(current_sprite_ == nullptr){
        //     SPDLOG_ERROR("Current sprite must be set prior to enabling this editor!");
        //     return;
        // }
        enabled_ = true;
        //current_sprite_->registerPickers();
        //add interesting events to interactor, so you can get event notification.
        event_translator_->addEventsToInteractor(current_interactor_, event_callback_cmd_, event_priority_);
        // broadcast enalbe state to others! 
        InvokeEvent(vtkCommand::EnableEvent, nullptr);
        if(current_sprite_){
            current_sprite_->startEditing({});
        }        
    }else{
        SPDLOG_TRACE("Disabling sprite editor({})...", GetClassName());
        if(!enabled_){ // do nothing if already disabled!
            return;
        }
        enabled_ = false;
        // remove event callback observer
        current_interactor_->RemoveObserver(event_callback_cmd_);
        // broadcast disable state to others!
        InvokeEvent(vtkCommand::DisableEvent, nullptr);
        // if(current_sprite_){
        //     current_sprite_->unRegisterPickers();
        // }
        if(current_sprite_){
            current_sprite_->endEditing({});
        }
    }
}

void Editor::ProcessEventHandler(vtkObject* object, unsigned long vtk_event, void* client_data, void* call_data)
{
    auto self = reinterpret_cast<Editor*>(client_data);
    if(self == nullptr){
        self->continuePassEventToNext();
        return;
    }
    bool handled = self->handleEventHandler(object, vtk_event, client_data, call_data);
    //take serious consideration if doing this is Reasonable?
    if(!handled){
        self->continuePassEventToNext(); 
    }else{
        self->continuePassEventToNext(false); 
    }
}

bool Editor::handleEventHandler(vtkObject* object, unsigned long vtk_event, void* client_data, void* call_data)
{
    if(!enabled()){
        return false;
    }    
    auto event_x = current_interactor_->GetEventPosition()[0];
    auto event_y = current_interactor_->GetEventPosition()[1];
    auto pocked_scene = findPockedScene(event_x, event_y);
    //only process event occur at allowed scenes.
    if(!isAllowedScene(pocked_scene)){
        return false;
    }
    //change current scene object 
    current_scene_ = pocked_scene;

    // if the event has data then get the translation using the event data
    unsigned long action_event = ActionEvent::NoEvent;
    if(call_data && vtkCommand::EventHasData(vtk_event)){
        action_event = event_translator_->getTranslation(vtk_event, static_cast<vtkEventData*>(call_data));
    }else{
        int modifier = vtkEvent::GetModifier(current_interactor_);
        // If neither the ctrl nor the shift keys are pressed, give NoModifier a preference over AnyModifer.
        if (modifier == vtkEvent::AnyModifier){
            action_event = event_translator_->getTranslation(
                vtk_event, 
                vtkEvent::NoModifier, 
                current_interactor_->GetKeyCode(),  
                current_interactor_->GetRepeatCount(),
                current_interactor_->GetKeySym()
            );
        }
        if(action_event == ActionEvent::NoEvent){
            action_event = event_translator_->getTranslation(
                vtk_event, 
                modifier, 
                current_interactor_->GetKeyCode(),
                current_interactor_->GetRepeatCount(), 
                current_interactor_->GetKeySym()
            );
        }
    }
    // Save the call data for widgets if needed
    event_call_data_ = call_data;
    // Invoke the widget callback
    if(action_event != ActionEvent::NoEvent){
        action_slot_mapper_.invoke(action_event);
        return true;
    }
    return false;
}

Scene* Editor::findPockedScene(int x, int y)const
{
    if(current_interactor_){
        auto ren = current_interactor_->FindPokedRenderer(x, y);
        return ren ? RenderWidget::RendererToScene(ren) : nullptr;
    }
    return nullptr;    
}

void Editor::setDisplayScenes(const SceneList& scenes)
{
    display_scenes_ = scenes;
}

NAMESPACE_END
