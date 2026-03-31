/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/9/10
*******************************************************/
#include "surgical_path_editor.h"
#include <vtkVolumePicker.h>
#include <vtkObjectFactory.h>
#include "surgical_path_sprite.h"
#include "mirfak/sprites/sphere_sprite.h"
#include "mirfak/sprites/line_sprite.h"
#include "mirfak/sprites/abstract_volume_sprite.h"
#include "mirfak/basic/log_service.h"
#include "mirfak/core/scene.h"
#include "mirfak/core/render_widget.h"

MIRFAK_NAMESPACE_BEGIN

vtkStandardNewMacro(SurgicalPathEditor);

namespace{
    int gNextPathId = 1;
}

SurgicalPathEditor::SurgicalPathEditor()
{
    action_slot_mapper_.setActionSlot(
        vtkCommand::LeftButtonPressEvent,
        ActionEvent::Select,
        this,
        SurgicalPathEditor::SelectAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::LeftButtonReleaseEvent,
        ActionEvent::EndSelect,
        this, 
        SurgicalPathEditor::EndSelectAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::MouseMoveEvent,
        ActionEvent::Move,
        this, 
        SurgicalPathEditor::MovePointAction
    );
}

SurgicalPathEditor::~SurgicalPathEditor()
{
}

bool SurgicalPathEditor::setCurrentSprite(Sprite* s)
{
    auto path = SurgicalPathSprite::SafeDownCast(s);
    if(path == nullptr){
        SPDLOG_WARN("Invalid 'SurgicalPathSprite' object pointer passed!");
        return false;
    }
    current_path_ = path;
    return Editor::setCurrentSprite(s);
}

void SurgicalPathEditor::setCurrentVolume(AbstractVolumeSprite* v)
{
    current_volume_ = v;
}

void SurgicalPathEditor::SelectAction(Editor* e)
{
    auto self = reinterpret_cast<SurgicalPathEditor*>(e);
    self->handleSelectAction();
}

void SurgicalPathEditor::EndSelectAction(Editor* e)
{
    auto self = reinterpret_cast<SurgicalPathEditor*>(e);
    self->handleUnselectAction();
}

void SurgicalPathEditor::MovePointAction(Editor* e)
{
    auto self = reinterpret_cast<SurgicalPathEditor*>(e);
    self->handleMovePointAction();
}

void SurgicalPathEditor::handleSelectAction()
{
    left_mouse_key_down_ = true;
    SPDLOG_TRACE("handleSelectAction");    
    int x, y;
    current_interactor_->GetEventPosition(x, y);    
    auto picked_wpos = pickWorldPoint(x, y);
    if(ea_ == EA_CREATE_PATH){        
        if(editor_state_ == ES_START){
            if(!picked_wpos){
                return;
            }
            Point3 future_end_point = *picked_wpos;
            if(current_created_path_ == nullptr){
                auto new_path = std::make_unique<SurgicalPathSprite>(1.0);
                new_path->setName(std::format("Path{:02}", gNextPathId++));
                new_path->setColor(Color::Purple);
                current_created_path_ = new_path.get();
                path_group_->append(std::move(new_path));
                current_created_path_->setEndPoint(future_end_point);         
                current_created_path_->connectScenes(display_scenes_);
            }
            editor_state_ = ES_DEFINE;
            return;
        } 
        if(editor_state_ == ES_DEFINE){
            if(!picked_wpos){
                return;
            }
            if(current_created_path_){
                current_created_path_->setStartPoint(*picked_wpos);
                std::string name = current_created_path_->name();
                PathCreatedSignal.invoke(name);
            }
            current_scene_->render();        
            current_created_path_ = nullptr;
            editor_state_ = ES_START;
            setEnabled(false); //Only support once operation on creation mode!
        }
        return;
    }  
    last_move_point_ = picked_wpos;
}

void SurgicalPathEditor::handleUnselectAction()
{
    left_mouse_key_down_ = false;

}

void SurgicalPathEditor::handleTranslateAction()
{

}

void SurgicalPathEditor::handleEndTranslateAction()
{

}

void SurgicalPathEditor::handleMovePointAction()
{
    if(ea_ == EA_CREATE_PATH && editor_state_ == ES_DEFINE){
        int x, y;
        current_interactor_->GetEventPosition(x, y);    
        auto picked_wpos = pickWorldPoint(x, y);
        if(picked_wpos && current_created_path_){
            current_created_path_->setStartPoint(*picked_wpos);
            path_group_->setCurrentSprite(current_created_path_);
            current_scene_->render();
        }
        return;
    }
    if(ea_ == EA_EDIT_PATH && editor_state_ == ES_START && current_sprite_){
		int x, y;
		current_interactor_->GetEventPosition(x, y);
        if(!left_mouse_key_down_){
            int state = current_sprite_->computeEditState(*current_scene_, x, y);
            updateCursorStyle(state);
            return;
        }
        auto picked_wpos = pickWorldPoint(x, y);
        if(!picked_wpos.has_value()){
            return;
        }
        auto state = current_sprite_->getEditState();
        if(state == SurgicalPathSprite::ES_START_POINT_SELECTED){
            current_path_->setStartPoint(picked_wpos.value());
        }else if(state == SurgicalPathSprite::ES_END_POINT_SELECTED){
            current_path_->setEndPoint(picked_wpos.value());
        }else if(state == SurgicalPathSprite::ES_HOVER){
            if(!last_move_point_.has_value()){
                return;
            }
            auto future_start_pt = current_path_->startPoint();
            auto future_end_pt = current_path_->endPoint();
            Vec3 move_vec = *picked_wpos - *last_move_point_;
            future_start_pt += move_vec;
            future_end_pt += move_vec;
            current_path_->setPoints(future_start_pt, future_end_pt);
            last_move_point_ = picked_wpos;
        }                
        current_scene_->render();
        return;
    }
}

Pt3Opt SurgicalPathEditor::pickWorldPoint(int x, int y)
{
    Pt3Opt pt;
    if(current_volume_){
        if(current_volume_->checkConnective(*current_scene_)){
            pt = current_volume_->pickWorldPoint(x, y, *current_scene_);
        }
        /*StopWatch sw;
        pt = current_volume_->pickWorldPoint(x, y, *current_scene_);*/
        //if(pt){
        //    SPDLOG_DEBUG("vol picked! {}", pt.value().toStr());
        //    SPDLOG_DEBUG("pick cost time:{} ms", sw.elapsedMillisecs());
        //}
        
    }
    if(!pt){
        pt = current_scene_->grabWorldPointFromPixel(x, y);
        //if(pt){
            //SPDLOG_DEBUG("grab pixel picked! {}", pt.value().toStr());
        //}
    }
    // if(!pt){
    //     SpriteCellPicker scp;
    //     scp.setExcludedSprites({current_polygon_});
    //     auto pinfos = scp.pick(*current_scene_, x, y);
    //     if(!pinfos.empty()){
    //         pt = pinfos[0].position;
    //         SPDLOG_DEBUG("SpriteCellPicker picked! pt:{} sprite:{}", pinfos[0].position.toStr(), pinfos[0].sprite->getClassName());
    //     }
    //     // pt = current_scene_->grabWorldPointFromPixel(x, y);
    //     // if(pt){
    //     //     SPDLOG_DEBUG("grab picked! pt:{}", pt.value().toStr());
    //     // }
    // }
    return pt;
}

void SurgicalPathEditor::updateCursorStyle(int state)
{
    if(currentInteractor() == nullptr){
        return;
    }
    Qt::CursorShape shape;   
    switch(state){
        case SurgicalPathSprite::ES_OUTSIDE:
            shape = Qt::ArrowCursor;
            break;
        case SurgicalPathSprite::ES_START_POINT_SELECTED:
            shape = Qt::ClosedHandCursor;
            break;
        case SurgicalPathSprite::ES_END_POINT_SELECTED:
            shape = Qt::OpenHandCursor;
            break;
        case SurgicalPathSprite::ES_HOVER:
            shape = Qt::SizeAllCursor;
            break;
        default:
            shape = Qt::ArrowCursor;
            break;
    };
    current_scene_->sceneWidget()->setCursor(shape);
}

NAMESPACE_END