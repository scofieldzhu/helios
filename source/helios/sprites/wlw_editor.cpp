/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/9/2
*******************************************************/
#include "wlw_editor.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkObjectFactory.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>
#include "helios/basic/log_service.h"
#include "helios/core/plane_scene.h"
#include "helios/sprites/ortho_planes_sprite.h"
#include "helios/sprites/surface_volume_sprite.h"

HELIOS_NAMESPACE_BEGIN

vtkStandardNewMacro(WLWEditor);

WLWEditor::WLWEditor()
{
    action_slot_mapper_.setActionSlot(
        vtkCommand::LeftButtonPressEvent,
        ActionEvent::Select,
        this,
        WLWEditor::SelectAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::LeftButtonReleaseEvent,
        ActionEvent::EndSelect,
        this, 
        WLWEditor::EndSelectAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::MouseMoveEvent,
        ActionEvent::Move3D,
        this,
        WLWEditor::Move3DAction
    );
}

WLWEditor::~WLWEditor()
{

}

void WLWEditor::SelectAction(Editor* e)
{
    auto self = reinterpret_cast<WLWEditor*>(e);
    self->handleSelectAction();
}

void WLWEditor::EndSelectAction(Editor* e)
{
    auto self = reinterpret_cast<WLWEditor*>(e);
    self->handleUnselectAction();
}

void WLWEditor::Move3DAction(Editor* e)
{
    auto self = reinterpret_cast<WLWEditor*>(e);
    self->handleMove3DAction();
}

void WLWEditor::handleSelectAction()
{
    if(state_ == ES_IDLE){
        state_ = ES_WORKING;
        int x, y;
        current_interactor_->GetEventPosition(x, y);  
        last_event_x_ = x;
        last_event_y_ = y;
    }
}

void WLWEditor::handleUnselectAction()
{
    if(state_ == ES_WORKING){
        state_ = ES_IDLE;
    }
}

void WLWEditor::handleMove3DAction()
{
    if(state_ == ES_WORKING){        
        int x, y;
        current_interactor_->GetEventPosition(x, y);  
        double delta_ww = x - last_event_x_;
        double delta_wl = y - last_event_y_;
        if(delta_ww == 0 && delta_wl == 0){
            return;
        }
        double new_wl = 0;
        double new_ww = 0;        
        if(PlaneScene::SafeDownCast(current_scene_) == nullptr && volume_){
            auto volume_scalar_range = volume_->getInput()->GetScalarRange();
            auto current_threshold_range = volume_->getThresholdRange();
            double min_v = current_threshold_range[0];
            double max_v = current_threshold_range[1];            
            double current_wl = (int)((max_v + min_v) / 2);
            double current_ww = max_v - min_v;
            new_wl = current_wl + delta_wl;
            new_ww = current_ww + delta_ww;
            if(new_ww < 2 || new_wl <= volume_scalar_range[0] || new_wl >= volume_scalar_range[1]){
                SPDLOG_WARN("new wlw is invalid, neglected! new_ww:{} new_wl:{}", new_ww, new_wl);
                return;
            }
            double low = new_wl - ((int)new_ww) / 2;
            double high = new_wl + ((int)new_ww) / 2;
            if(low < volume_scalar_range[0]){
                low = volume_scalar_range[0];
            }
            if(high > volume_scalar_range[1]){
                high = volume_scalar_range[1];
            }
            volume_->setVolumeThresholdValue(100, low, high);
            //SPDLOG_TRACE("scene({}): current_wl:{} current_ww:{} low:{} hight:{}", current_scene_->name(), current_wl, current_ww, low, high);
        }else if(PlaneScene::SafeDownCast(current_scene_) && orthoplanes_){
            auto volume_scalar_range = orthoplanes_->getInputData()->GetScalarRange();
            auto lookup_table = orthoplanes_->getLookupTable(0);
            double min_v = lookup_table->GetTableRange()[0];
            double max_v = lookup_table->GetTableRange()[1];
            double current_wl = (int)((max_v + min_v) / 2);
            double current_ww = max_v - min_v;
            new_wl = current_wl + delta_wl;
            new_ww = current_ww + delta_ww;
            if(new_ww < 2 || new_wl <= volume_scalar_range[0] || new_wl >= volume_scalar_range[1]){
                SPDLOG_WARN("new wlw is invalid, neglected! new_ww:{} new_wl:{}", new_ww, new_wl);
                return;
            }
            double low = new_wl - ((int)new_ww) / 2;
            double high = new_wl + ((int)new_ww) / 2;
            if(low < volume_scalar_range[0]){
                low = volume_scalar_range[0];
            }
            if(high > volume_scalar_range[1]){
                high = volume_scalar_range[1];
            }
            lookup_table->SetTableRange(low, high);
            lookup_table->Modified();
            orthoplanes_->modified();
            //SPDLOG_TRACE("scene({}): current_wl:{} current_ww:{} low:{} hight:{}", current_scene_->name(), current_wl, current_ww, low, high);
        }
        current_scene_->render();
        if(new_wl > 0.0 || new_ww > 0.0){
            call_data_.event_scene = current_scene_;
            call_data_.wl = new_wl;
            call_data_.ww = new_ww;
            InvokeEvent(WLW_UPDATE_EVENT, &call_data_);
        }
        last_event_x_ = x;
        last_event_y_ = y;
    }
}

// bool ChangeThresholdRangeEditor::setCurrentSprite(Sprite *s)
// {
//     if(s == current_sprite_){
//         return true;
//     }
//     if(s && SurfaceVolumeSprite::SafeDownCast(s)){
//         current_sprite_ = s;
//         volume_sprite_ = SurfaceVolumeSprite::SafeDownCast(s);
//         return true;
//     }
//     return false;
// }

void WLWEditor::setOrthoPlanes(helios::OrthoPlanesSprite* ortho_planes)
{
    orthoplanes_ = ortho_planes;
}

void WLWEditor::setVolume(helios::SurfaceVolumeSprite* v)
{
    volume_ = v;
}

NAMESPACE_END