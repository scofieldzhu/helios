/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/10/31
*******************************************************/
#include "point_move_tool_editor.h"
#include <vtkObjectFactory.h>
#include <vtkTransform.h>
#include <vtkMath.h>
#include "mirfak/core/render_widget.h"
#include "mirfak/basic/log_service.h"

MIRFAK_NAMESPACE_BEGIN

vtkStandardNewMacro(PointMoveToolEditor);

PointMoveToolEditor::PointMoveToolEditor()
{
    action_slot_mapper_.setActionSlot(
        vtkCommand::LeftButtonPressEvent,
        ActionEvent::Select,
        this,
        PointMoveToolEditor::SelectAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::LeftButtonReleaseEvent,
        ActionEvent::EndSelect,
        this, 
        PointMoveToolEditor::EndSelectAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::MouseMoveEvent,
        ActionEvent::Move,
        this, 
        PointMoveToolEditor::MoveAction
    );
}

PointMoveToolEditor::~PointMoveToolEditor()
{

}

bool PointMoveToolEditor::setCurrentSprite(Sprite* s)
{
	auto pmt = PointMoveToolSprite::SafeDownCast(s);
	if(pmt == nullptr){
		SPDLOG_ERROR("The source sprite is not object with class type 'PointMoveToolSprite'.");
		return false;
	}
	move_tool_ = pmt;
	Editor::setCurrentSprite(s);
	return true;
}

void PointMoveToolEditor::SelectAction(Editor* e)
{
    auto self = reinterpret_cast<PointMoveToolEditor*>(e);
    self->handleSelectAction();
}

void PointMoveToolEditor::EndSelectAction(Editor* e)
{
    auto self = reinterpret_cast<PointMoveToolEditor*>(e);
    self->handleUnselectAction();
}

void PointMoveToolEditor::MoveAction(Editor* e)
{
    auto self = reinterpret_cast<PointMoveToolEditor*>(e);
    self->handleMoveAction();
}

void PointMoveToolEditor::handleSelectAction()
{
    left_button_down_ = true;
    editor_state_ = ES_START;
    if(move_tool_ == nullptr){
        return;
    }
    if(move_tool_->getEditState() <= PointMoveToolSprite::ES_OUTSIDE){
        return;
    }
    start_drag_data_ = *static_cast<const PointMoveToolSprite::EditStateData*>(move_tool_->getEditStateData());    
    if(start_drag_data_.state == PointMoveToolSprite::ES_AXIS_X_TRANSLATE ||
        start_drag_data_.state == PointMoveToolSprite::ES_AXIS_Y_TRANSLATE ||
        start_drag_data_.state == PointMoveToolSprite::ES_AXIS_Z_TRANSLATE){
        auto c = move_tool_->center();
        auto move_dir = (start_drag_data_.translate_axis_normal.value()).normalized();
        last_drag_point_ = Line(c, move_dir).projectPoint(*start_drag_data_.world_point);
        editor_state_ = ES_TRANSLATING;
    }else{
        last_drag_point_ = start_drag_data_.world_point;//Line(c, plane_normal).projectPoint(*start_drag_data_.world_point);
        editor_state_ = ES_ROTATING;
    }
}

void PointMoveToolEditor::handleUnselectAction()
{
    left_button_down_ = false;
    editor_state_ = ES_START;
}

void PointMoveToolEditor::handleTranslateAction()
{

}

void PointMoveToolEditor::handleEndTranslateAction()
{

}

void PointMoveToolEditor::handleMoveAction()
{
    if(editor_state_ == ES_START && move_tool_){
        int x, y;
        current_interactor_->GetEventPosition(x, y);    
        auto s =  move_tool_->computeEditState(*current_scene_, x, y);
        //SPDLOG_DEBUG("Current edit state:{}", s);
        updateCursorStyle(move_tool_->getEditState());
        return;
    }
    if(editor_state_ > ES_START && move_tool_ && left_button_down_){
        int x, y;
        current_interactor_->GetEventPosition(x, y);  
        auto picked_pt = pickWorldPoint(x, y);
        if(!picked_pt){
            SPDLOG_DEBUG("No world point picked!");
            editor_state_ = ES_START;
            return;
        }     
        if(editor_state_ == ES_TRANSLATING){                        
            doTranslate(*picked_pt);
        }else if(editor_state_ == ES_ROTATING){        
            doRotate(*picked_pt);
        }
        move_tool_->render();
        return;
    }
}

void PointMoveToolEditor::updateCursorStyle(int state)
{
    if(currentInteractor() == nullptr){
        return;
    }
    Qt::CursorShape shape;   
    if(state == PointMoveToolSprite::ES_AXIS_X_TRANSLATE){
        shape = Qt::SizeAllCursor;
    }else if(state == PointMoveToolSprite::ES_AXIS_Y_TRANSLATE){
        shape = Qt::SizeAllCursor;
    }else if(state == PointMoveToolSprite::ES_AXIS_Z_TRANSLATE){
        shape = Qt::SizeAllCursor;
    }else if(state == PointMoveToolSprite::ES_AXIS_X_ROTATE){
        shape = Qt::ClosedHandCursor;
    }else if(state == PointMoveToolSprite::ES_AXIS_Y_ROTATE){
        shape = Qt::ClosedHandCursor;
    }else if(state == PointMoveToolSprite::ES_AXIS_Z_ROTATE){
        shape = Qt::ClosedHandCursor;
    }else{
        shape = Qt::ArrowCursor;
    }
    current_scene_->sceneWidget()->setCursor(shape);
}

Pt3Opt PointMoveToolEditor::pickWorldPoint(int x, int y)
{
    auto pick_wpt = current_scene_->grabWorldPointFromPixel(x, y);
    if(pick_wpt){
        return pick_wpt;
    }
    //if(current_volume_){
    //    pick_wpt = current_volume_->pickWorldPoint(x, y, *current_scene_);
    //    if(pick_wpt){
    //        return pick_wpt;
    //    }
    //}
    return std::nullopt;
}

void PointMoveToolEditor::setEnabled(bool e)
{
    Editor::setEnabled(e);
    if(e){
        editor_state_ = ES_START;
    }else{
        editor_state_ = ES_NONE;
    }
}

void PointMoveToolEditor::doTranslate(const Point3& picked_pos)
{
    auto edit_state_data = static_cast<const PointMoveToolSprite::EditStateData*>(move_tool_->getEditStateData());
    auto tool_edit_state = static_cast<PointMoveToolSprite::EditState>(edit_state_data->state);
    auto c = move_tool_->center();    
    auto move_dir = (edit_state_data->translate_axis_normal.value()).normalized();
    auto final_pick_wpt = Line(c, move_dir).projectPoint(picked_pos);
    auto move_vec = final_pick_wpt - *last_drag_point_;
    if(move_vec.norm() > 0.1){                
        move_tool_->setCenter(c + move_vec);
        last_drag_point_ = final_pick_wpt;
    }  
}

void PointMoveToolEditor::doRotate(const Point3& picked_pt)
{
    auto edit_state_data = static_cast<const PointMoveToolSprite::EditStateData*>(move_tool_->getEditStateData());
    auto tool_edit_state = static_cast<PointMoveToolSprite::EditState>(edit_state_data->state);
    auto c = move_tool_->center();    
    Vec3 cur_vec = picked_pt - c;
    auto dist = picked_pt.distanceTo(c);
    Vec3 last_vec = last_drag_point_.value() - c;
    Vec3 cross_vec = cur_vec.crossed(last_vec);
    double rotate_angle = cur_vec.angle(last_vec);
    auto dot_v = cross_vec.dot(edit_state_data->rotate_axis_normal.value());
    rotate_angle *= (dot_v > 0 ? -1.0 : 1.0);
    SPDLOG_DEBUG("Rotate angle:{} tool_edit_state:{} picked_pt:{} drag_pt:{}", rotate_angle, (int)tool_edit_state, picked_pt.toStr(), last_drag_point_.value().toStr());
    Point3 p1, p2;
    Vec3 move_dir1, move_dir2;
    if(tool_edit_state == PointMoveToolSprite::ES_AXIS_X_ROTATE){
        move_dir1 = move_tool_->getAxisYDirection();
        move_dir2 = move_tool_->getAxisZDirection();
    }else if(tool_edit_state == PointMoveToolSprite::ES_AXIS_Y_ROTATE){
        move_dir1 = move_tool_->getAxisXDirection();
        move_dir2 = move_tool_->getAxisZDirection();
    }else if(tool_edit_state == PointMoveToolSprite::ES_AXIS_Z_ROTATE){
        move_dir1 = move_tool_->getAxisXDirection();
        move_dir2 = move_tool_->getAxisYDirection();
    }
    Point3 translate_axis_point1 = c + move_dir1.normalized();
    Point3 translate_axis_point2 = c + move_dir2.normalized();
    vtkNew<vtkTransform> t;
    t->PostMultiply();
    t->Identity();
    t->Translate(-c[0], -c[1], -c[2]);
    t->RotateWXYZ(rotate_angle, edit_state_data->rotate_axis_normal.value());
    t->Translate(c[0], c[1], c[2]);
    Point3 after_rotate_point1 = t->TransformPoint(translate_axis_point1);
    Point3 after_rotate_point2 = t->TransformPoint(translate_axis_point2);
    auto new_dir1 = (after_rotate_point1 - c).normalized();
    auto new_dir2 = (after_rotate_point2 - c).normalized();
    if(tool_edit_state == PointMoveToolSprite::ES_AXIS_X_ROTATE){
        move_tool_->setAxisYDirection(new_dir1);
        move_tool_->setAxisZDirection(new_dir2);
    }else if(tool_edit_state == PointMoveToolSprite::ES_AXIS_Y_ROTATE){
        move_tool_->setAxisXDirection(new_dir1);
        move_tool_->setAxisZDirection(new_dir2);
    }else if(tool_edit_state == PointMoveToolSprite::ES_AXIS_Z_ROTATE){
        move_tool_->setAxisXDirection(new_dir1);
        move_tool_->setAxisYDirection(new_dir2);
    }
    last_drag_point_ = picked_pt;
}

NAMESPACE_END