/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/6/26
*******************************************************/
#include "plane_intersection_line_editor.h"
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCommand.h>
#include <vtkTransform.h>
#include <vtkCallbackCommand.h>
#include "plane_intersection_line_sprite.h"
#include "plane_cutter_sprite.h"
#include "helios/basic/log_service.h"
#include "helios/core/scene.h"
#include "helios/core/render_widget.h"
#include "helios/core/slice_plane_sprite.h"
#include "helios/basic/image_data_util.h"

HELIOS_NAMESPACE_BEGIN

vtkStandardNewMacro(PlaneIntersectionLineEditor);

PlaneIntersectionLineEditor::PlaneIntersectionLineEditor()
{
    action_slot_mapper_.setActionSlot(
        vtkCommand::LeftButtonPressEvent,
        ActionEvent::Select,
        this, 
        PlaneIntersectionLineEditor::SelectAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::LeftButtonReleaseEvent,
        ActionEvent::EndSelect,
        this, 
        PlaneIntersectionLineEditor::EndSelectAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::MouseMoveEvent,
        ActionEvent::Move,
        this, 
        PlaneIntersectionLineEditor::MoveAction
    );
}

PlaneIntersectionLineEditor::~PlaneIntersectionLineEditor()
{
}

bool PlaneIntersectionLineEditor::setCurrentSprite(Sprite* s)
{
    if(s == current_sprite_){
        return true;
    }
    if(s && PlaneIntersectionLineSprite::SafeDownCast(s)){
        current_sprite_ = s;
        pil_ = PlaneIntersectionLineSprite::SafeDownCast(s);
        return true;
    }
    return false;
}

void PlaneIntersectionLineEditor::updateCursorStyle(int state)
{
    if(currentInteractor() == nullptr){
        return;
    }
    auto rw = currentInteractor()->GetRenderWindow();
    Qt::CursorShape shape;
    auto sd = reinterpret_cast<const PlaneIntersectionLineSprite::InteractionStateData*>(pil_->getEditStateData());
    switch(state){
        case PlaneIntersectionLineSprite::IS_OUTSIDE:
            shape = Qt::ArrowCursor;
            break;
        case PlaneIntersectionLineSprite::IS_TRANSLATE_H:
            {
                auto abs_angle = fabs(sd->angle);
                if(abs_angle > 0 && abs_angle <= 25.0){
                    shape = Qt::SizeVerCursor;
                }else if(abs_angle > 25.0 && abs_angle < 60.0){
                    shape = (sd->angle > 0.0 ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);
                }else{
                    shape = Qt::SizeHorCursor;    
                }
            }
            break;
        case PlaneIntersectionLineSprite::IS_TRANSLATE_V:
            {
                auto abs_angle = fabs(sd->angle);
                if(abs_angle > 0 && abs_angle <= 25.0){
                    shape = Qt::SizeHorCursor;
                }else if(abs_angle > 25.0 && abs_angle < 60.0){
                    shape = (sd->angle > 0.0 ? Qt::SizeBDiagCursor : Qt::SizeFDiagCursor);
                }else{
                    shape = Qt::SizeVerCursor;    
                }
            }
            break;
        case PlaneIntersectionLineSprite::IS_MOVE_CENTER:
            shape = Qt::SizeAllCursor;
            break;            
        case PlaneIntersectionLineSprite::IS_ROTATING:
            shape = Qt::ClosedHandCursor;
            break;
        default:
            shape = Qt::ArrowCursor;
            break;
    };
    current_scene_->sceneWidget()->setCursor(shape);
}

void PlaneIntersectionLineEditor::SelectAction(Editor* e)
{
    auto self = reinterpret_cast<PlaneIntersectionLineEditor*>(e);
    self->handleSelectAction();
}

void PlaneIntersectionLineEditor::handleSelectAction()
{
    if(pil_ == nullptr){
        return;
    }
    int x = currentInteractor()->GetEventPosition()[0];
    int y = currentInteractor()->GetEventPosition()[1];
    auto interaction_state = pil_->getEditState();
    //SPDLOG_DEBUG("SelectAction... state:{} event_priority:{}", editor_state_, event_priority_);
    if(interaction_state == PlaneIntersectionLineSprite::IS_OUTSIDE){
        return;
    }
    if(interaction_state == PlaneIntersectionLineSprite::IS_ROTATING){
        editor_state_ = ES_ROTATING;
        onRotateStarted({x, y});
        return;
    }
    if(interaction_state == PlaneIntersectionLineSprite::IS_TRANSLATE_H){
        editor_state_ = ES_TRANSLATING_H;
        onTranslateHStarted({x, y});
        return;
    }
    if(interaction_state == PlaneIntersectionLineSprite::IS_TRANSLATE_V){
        editor_state_ = ES_TRANSLATING_V;
        onTranslateVStarted({x, y});
        return;
    }    
    if(interaction_state == PlaneIntersectionLineSprite::IS_MOVE_CENTER){
        editor_state_ = ES_MOVING_CENTER;
        onMoveCenterStarted({x, y});
        return;
    }  
}

void PlaneIntersectionLineEditor::EndSelectAction(Editor* e)
{
    auto self = reinterpret_cast<PlaneIntersectionLineEditor*>(e);
    self->handleEndSelectAction();
}

void PlaneIntersectionLineEditor::handleEndSelectAction()
{
    if(pil_ == nullptr){
        return;
    }
    int x = currentInteractor()->GetEventPosition()[0];
    int y = currentInteractor()->GetEventPosition()[1];  
    if(editor_state_ != ES_START){
        if(editor_state_ == ES_ROTATING){
            onRotateEnd({x, y});
        }else if(editor_state_ == ES_TRANSLATING_H){
            onTranslateHEnd({x, y});
        }else if(editor_state_ == ES_TRANSLATING_V){
            onTranslateVEnd({x, y});
        }else if(editor_state_ == ES_MOVING_CENTER){
            onMoveCenterEnd({x, y});
        }
    }  
    editor_state_ = ES_START;
    if(current_scene_ == nullptr){
        return;
    }
    auto s = pil_->computeEditState(*current_scene_, x, y);    
    updateCursorStyle(s);
    //SPDLOG_TRACE("EndSelectAction... state:{} event_priority:{}", editor_state_, event_priority_);
}

void PlaneIntersectionLineEditor::MoveAction(Editor* e)
{
    auto self = reinterpret_cast<PlaneIntersectionLineEditor*>(e);
    self->handleMoveAction();
}

void PlaneIntersectionLineEditor::handleMoveAction()
{
    if(pil_ == nullptr){
        return;
    }
    //SPDLOG_TRACE("MoveAction... state:{} event_priority:{}", editor_state_, event_priority_);
    int x = currentInteractor()->GetEventPosition()[0];
    int y = currentInteractor()->GetEventPosition()[1];    
    if(editor_state_ == ES_START){
        auto s = pil_->computeEditState(*current_scene_, x, y);    
        updateCursorStyle(s);
        return;
    }
    if(editor_state_ == ES_ROTATING){
        //the editor is in dragging state!
        onRotating({x, y});
    }else if(editor_state_ == ES_TRANSLATING_H){
        onTranslatingH({x, y});
    }else if(editor_state_ == ES_TRANSLATING_V){
        onTranslatingV({x, y});
    }else if(editor_state_ == ES_MOVING_CENTER){
        onMovingCenter({x, y});
    }
}

void PlaneIntersectionLineEditor::onRotateStarted(const Point2i& pos)
{
    auto sd = reinterpret_cast<const PlaneIntersectionLineSprite::InteractionStateData*>(pil_->getEditStateData());
    if(sd == nullptr){
        //bad state data object!
        return;
    }
    auto target_plane = sd->on_plane;
    drag_state_info_ = *sd;
    Point3 display_pos((double)pos.x, (double)pos.y, 0.0);
    auto wpos = sd->scene->displayToWorld(display_pos);
    Plane proj_plane(target_plane->getOrigin(), target_plane->getNormal());
    last_drag_wpos_ = proj_plane.projectPoint(wpos);
}

void PlaneIntersectionLineEditor::onRotating(const Point2i& pos)
{
    Point3 last_dpos = drag_state_info_.scene->worldToDisplay(last_drag_wpos_);
    //? check mouse offset if at minimum movement
    Point3 display_pos((double)pos.x, (double)pos.y, 0.0);
    if(display_pos == last_dpos){ // redundant mouse point passed
        return;
    }
    auto current_drag_pos = drag_state_info_.scene->displayToWorld(display_pos);
    Plane proj_plane(drag_state_info_.on_plane->getOrigin(), drag_state_info_.on_plane->getNormal());
    current_drag_pos = proj_plane.projectPoint(current_drag_pos);

    Point3 ortho_center = calcCurrentOrthoCenter();
    auto rotate_angle = calcRotateAngleOnPlane(current_drag_pos, last_drag_wpos_, ortho_center, drag_state_info_.on_plane->getNormal());
    if(fabs(rotate_angle) < 0.001){
        //too small to rotate it!
        return;
    }
    Plane p0(drag_state_info_.picked_plane->getOrigin(), drag_state_info_.picked_plane->getNormal());
    Plane p1(drag_state_info_.cross_plane->getOrigin(), drag_state_info_.cross_plane->getNormal());
    Line l = p0.intersectWithPlane(p1).value();
    auto n = l.normal.normalized();
    auto prefer_axis = drag_state_info_.on_plane->getNormal().normalized();
    if(n.dot(prefer_axis) < 0.0){
        n = -n;
    }
    l.normal = n;
    auto start = l.origin;
    auto normal = l.normal;
    vtkNew<vtkTransform> rotation_trans;
    rotation_trans->Identity();
    rotation_trans->Translate(start);
    rotation_trans->RotateWXYZ(rotate_angle, normal);
    rotation_trans->Translate(-start);

    auto cut_plane = drag_state_info_.picked_plane;
    double pos_rate = cut_plane->getPosition() / cut_plane->pushRange();
    Point3 new_p1 = rotation_trans->TransformPoint(cut_plane->getPoint1());
    Point3 new_p2 = rotation_trans->TransformPoint(cut_plane->getPoint2());
    Point3 new_origin = rotation_trans->TransformPoint(cut_plane->getOrigin());
    cut_plane->setOrigin(new_origin);
    cut_plane->setPoint1(new_p1);
    cut_plane->setPoint2(new_p2);

    auto data_tuple = CalcPushRangeAndStartPointOfImagePlane(cut_plane->getInputData(), cut_plane->getNormal());
    cut_plane->setStartPoint(std::get<Point3>(data_tuple));
    cut_plane->setPushRange(std::get<double>(data_tuple));
    cut_plane->updatePosition();

    auto cross_plane = drag_state_info_.cross_plane;
    pos_rate = cross_plane->getPosition() / cross_plane->pushRange();
    new_p1 = rotation_trans->TransformPoint(cross_plane->getPoint1());
    new_p2 = rotation_trans->TransformPoint(cross_plane->getPoint2());
    new_origin = rotation_trans->TransformPoint(cross_plane->getOrigin());
    cross_plane->setOrigin(new_origin);
    cross_plane->setPoint1(new_p1);
    cross_plane->setPoint2(new_p2);

    data_tuple = CalcPushRangeAndStartPointOfImagePlane(cross_plane->getInputData(), cross_plane->getNormal());
    cross_plane->setStartPoint(std::get<Point3>(data_tuple));
    cross_plane->setPushRange(std::get<double>(data_tuple));
    cross_plane->updatePosition();

    cut_plane->render();
    last_drag_wpos_ = current_drag_pos;

    InvokeEvent(ROTATING_EVENT, nullptr);

}

Point3 PlaneIntersectionLineEditor::calcCurrentOrthoCenter() const
{
    Plane p1(drag_state_info_.on_plane->getOrigin(), drag_state_info_.on_plane->getNormal());
    Plane p2(drag_state_info_.picked_plane->getOrigin(), drag_state_info_.picked_plane->getNormal());
    Plane p3(drag_state_info_.cross_plane->getOrigin(), drag_state_info_.cross_plane->getNormal());
    return Plane::CalcCenterOfTriplePlanes(p1, p2, p3).value();
}

double PlaneIntersectionLineEditor::calcRotateAngleOnPlane(const Point3& now_wpos, const Point3& last_wpos, const Point3& center, const Point3& normal) const
{
    Vec3 last_vec = last_wpos - center;
    last_vec.normalize();
    Vec3 now_vec = now_wpos - center;
    now_vec.normalize();
    if(now_vec.distanceTo(last_vec) < 0.01){
        return 0.0;
    }
    Vec3 cross_vec = last_vec;
    cross_vec.cross(now_vec);
    cross_vec.normalize();
    auto d = cross_vec.dot(normal.normalized());    
    auto rotate_angle = now_vec.angle(last_vec);
    rotate_angle = (d < 0 ? -rotate_angle : rotate_angle);
    return rotate_angle;
}

void PlaneIntersectionLineEditor::onRotateEnd(const Point2i& pos)
{
}

void PlaneIntersectionLineEditor::onTranslateHStarted(const Point2i &pos)
{
    auto sd = reinterpret_cast<const PlaneIntersectionLineSprite::InteractionStateData*>(pil_->getEditStateData());
    if(sd == nullptr){
        //bad state data object!
        return;
    }
    auto target_plane = sd->on_plane;
    drag_state_info_ = *sd;
    Point3 display_pos((double)pos.x, (double)pos.y, 0.0);
    auto wpos = sd->scene->displayToWorld(display_pos);
    Plane proj_plane(target_plane->getOrigin(), target_plane->getNormal());
    last_drag_wpos_ = proj_plane.projectPoint(wpos);
}

void PlaneIntersectionLineEditor::onTranslatingH(const Point2i &pos)
{
    Point3 display_pos((double)pos.x, (double)pos.y, 0.0);
    auto last_display_pos = drag_state_info_.scene->worldToDisplay(last_drag_wpos_);
    if(Point2(display_pos.x(), display_pos.y()) == Point2(last_display_pos.x(), last_display_pos.y())){
        SPDLOG_DEBUG("Redundant mouse point neglected!");
        return;
    }
    auto wpos = drag_state_info_.scene->displayToWorld(display_pos);
    Plane proj_plane(drag_state_info_.on_plane->getOrigin(), drag_state_info_.on_plane->getNormal());
    auto current_drag_wpos = proj_plane.projectPoint(wpos);
    if(movePlane(*drag_state_info_.picked_plane, current_drag_wpos)){
        drag_state_info_.picked_plane->render();
        last_drag_wpos_ = current_drag_wpos;
        InvokeEvent(TRANSLATING_EVENT, nullptr);
    }
}

void PlaneIntersectionLineEditor::onTranslateHEnd(const Point2i &pos)
{
}

void PlaneIntersectionLineEditor::onTranslateVStarted(const Point2i& pos)
{
    onTranslateHStarted(pos);
}

void PlaneIntersectionLineEditor::onTranslatingV(const Point2i &pos)
{
    onTranslatingH(pos);
}

void PlaneIntersectionLineEditor::onTranslateVEnd(const Point2i &pos)
{
}

void PlaneIntersectionLineEditor::onMoveCenterStarted(const Point2i &pos)
{
    auto sd = reinterpret_cast<const PlaneIntersectionLineSprite::InteractionStateData*>(pil_->getEditStateData());
    if(sd == nullptr){
        //bad state data object!
        return;
    }
    auto target_plane = sd->on_plane;
    drag_state_info_ = *sd;
    Point3 display_pos((double)pos.x, (double)pos.y, 0.0);
    auto wpos = sd->scene->displayToWorld(display_pos);
    Plane proj_plane(target_plane->getOrigin(), target_plane->getNormal());
    last_drag_wpos_ = proj_plane.projectPoint(wpos);
}

void PlaneIntersectionLineEditor::onMovingCenter(const Point2i &pos)
{
    Point3 display_pos((double)pos.x, (double)pos.y, 0.0);
    auto last_display_pos = drag_state_info_.scene->worldToDisplay(last_drag_wpos_);
    if(Point2(display_pos.x(), display_pos.y()) == Point2(last_display_pos.x(), last_display_pos.y())){
        SPDLOG_DEBUG("Redundant mouse point neglected!");
        return;
    }
    auto wpos = drag_state_info_.scene->displayToWorld(display_pos);
    Plane proj_plane(drag_state_info_.on_plane->getOrigin(), drag_state_info_.on_plane->getNormal());
    auto current_drag_wpos = proj_plane.projectPoint(wpos);
    bool ok = movePlane(*drag_state_info_.picked_plane, current_drag_wpos);
    ok = (movePlane(*drag_state_info_.cross_plane, current_drag_wpos) || ok);
    if(ok){
        drag_state_info_.picked_plane->render();
        last_drag_wpos_ = current_drag_wpos;
        InvokeEvent(TRANSLATING_EVENT, nullptr);
    }
}

bool PlaneIntersectionLineEditor::movePlane(SlicePlaneSprite& plane, const Point3 &drag_wpt)
{
    Plane proj_plane(plane.getOrigin(), plane.getNormal());
    auto current_proj_point = proj_plane.projectPoint(drag_wpt);
    auto future_move_dist = drag_wpt.distanceTo(current_proj_point);
    if(future_move_dist < 0.01){
        SPDLOG_DEBUG("Distance is too small to move!");
        return false;
    }
    auto move_vec = (drag_wpt - current_proj_point).normalized();
    if(move_vec.dot(proj_plane.normal.normalized()) < 0){
        future_move_dist = -future_move_dist;
    }
    plane.push(future_move_dist);
    return true;
}

void PlaneIntersectionLineEditor::onMoveCenterEnd(const Point2i &pos)
{
}

NAMESPACE_END