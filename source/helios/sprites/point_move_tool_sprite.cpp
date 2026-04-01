/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/10/30
*******************************************************/
#include "point_move_tool_sprite.h"
#include <vtkCellPicker.h>
#include <vtkAssemblyPath.h>
#include <vtkMatrix4x4.h>
#include "helios/core/scene.h"
#include "helios/basic/log_service.h"

HELIOS_NAMESPACE_BEGIN

PointMoveToolSprite::PointMoveToolSprite()
	:axis_x_(std::make_unique<ArrowSprite>(0.03, 0.08, 0.2)),
	axis_y_(std::make_unique<ArrowSprite>(0.03, 0.08, 0.2)),
	axis_z_(std::make_unique<ArrowSprite>(0.03, 0.08, 0.2)),
	torus_x_(std::make_unique<TorusSprite>(4.0, 6.0, 0.1)),
	torus_y_(std::make_unique<TorusSprite>(4.0, 6.0, 0.1)),
	torus_z_(std::make_unique<TorusSprite>(4.0, 6.0, 0.1))
{
	axis_x_->setColor(Color::Red);
	axis_x_->setStartPoint({0.0, 0.0, 0.0});
	axis_x_->setDirection({1.0, 0.0, 0.0});
	axis_x_->setLength(10);
	addChild(*axis_x_);

	axis_y_->setColor(Color::Green);
	axis_y_->setStartPoint({0.0, 0.0, 0.0});
	axis_y_->setDirection({0.0, 1.0, 0.0});
	axis_y_->setLength(10);
	addChild(*axis_y_);

	axis_z_->setColor(Color::Blue);
	axis_z_->setStartPoint({0.0, 0.0, 0.0});
	axis_z_->setDirection({0.0, 0.0, 1.0});
	axis_z_->setLength(10);
	addChild(*axis_z_);

	torus_x_->setColor(*axis_x_->getColor());
	torus_x_->setCenter(axis_x_->startPoint());
	torus_x_->setNormal(axis_x_->direction());
	addChild(*torus_x_);

	torus_y_->setColor(*axis_y_->getColor());
	torus_y_->setCenter(axis_y_->startPoint());
	torus_y_->setNormal(axis_y_->direction());
	addChild(*torus_y_);

	torus_z_->setColor(*axis_z_->getColor());
	torus_z_->setCenter(axis_z_->startPoint());
	torus_z_->setNormal(axis_z_->direction());
	addChild(*torus_z_);
}

PointMoveToolSprite::~PointMoveToolSprite()
{

}

void PointMoveToolSprite::createSource(Scene& scene)
{

}

void PointMoveToolSprite::makeActors(Scene& scene)
{

}

void PointMoveToolSprite::setCenter(const Point3& pt)
{
	axis_x_->setStartPoint(pt);
	axis_y_->setStartPoint(pt);
	axis_z_->setStartPoint(pt);
	torus_x_->setCenter(pt);
	torus_y_->setCenter(pt);
	torus_z_->setCenter(pt);
    CenterChanged.invoke(pt);
	modified();
}

const Point3& PointMoveToolSprite::center() const
{
    return axis_x_->startPoint();
}

void PointMoveToolSprite::setAxisXDirection(const Vec3& n)
{
	axis_x_->setDirection(n);
	torus_x_->setNormal(n);
	modified();
}

void PointMoveToolSprite::setAxisYDirection(const Vec3& n)
{
	axis_y_->setDirection(n);
	torus_y_->setNormal(n);
	modified();
}

void PointMoveToolSprite::setAxisZDirection(const Vec3& n)
{
	axis_z_->setDirection(n);
	torus_z_->setNormal(n);
	modified();
}

int PointMoveToolSprite::computeEditState(Scene& s, int x, int y, int modify /*= 0*/)
{
    edit_state_ = ES_NONE;
    edit_state_data_ = {};
    if(!isEditing() || !getVisibility(s)){        
        edit_state_data_.state = edit_state_;
        return edit_state_;
    }
    edit_state_ = ES_OUTSIDE;
    auto actors = getSceneActors(&s, true);    
    if(!actors->GetNumberOfItems()){
        edit_state_data_.state = edit_state_;
        return edit_state_;
    }
    if(!picker_dict_.contains(&s)){
        auto new_picker = PickerPtr::New();
        new_picker->SetPickFromList(true);
        actors->InitTraversal();
        while(auto a = actors->GetNextActor()){
            new_picker->AddPickList(a);
        }
        picker_dict_.insert({&s, new_picker});
    }
    auto target_picker = picker_dict_[&s];
    if(!target_picker->Pick(x, y, 0.0, s.getRenderer())){
        edit_state_data_.state = edit_state_;
        return edit_state_;
    }
    auto pick_data_tuple = std::make_tuple<vtkActor*, Pt3Opt>((vtkActor*)nullptr, std::nullopt);
    auto get_pick_data = [&pick_data_tuple](vtkAssemblyPath* path){
        auto top_node = path->GetFirstNode();
        auto top_actor = vtkActor::SafeDownCast(top_node->GetViewProp());
        if(top_actor){
            std::get<0>(pick_data_tuple) = top_actor;
            double x = top_node->GetMatrix()->GetElement(0, 3);
            double y = top_node->GetMatrix()->GetElement(1, 3);
            double z = top_node->GetMatrix()->GetElement(2, 3);
            std::get<1>(pick_data_tuple) = {x, y, z};
            return;
        }
        auto next_node = path->GetNextNode();
        while(next_node){
            if(auto a = vtkActor::SafeDownCast(next_node->GetViewProp())){
                std::get<0>(pick_data_tuple) = a;
                double x = next_node->GetMatrix()->GetElement(0, 3);
                double y = next_node->GetMatrix()->GetElement(1, 3);
                double z = next_node->GetMatrix()->GetElement(2, 3);
                std::get<1>(pick_data_tuple) = {x, y, z};
                break;
            }
            next_node = path->GetNextNode();
        }
    };    
    get_pick_data(target_picker->GetPath());
    auto picked_actor = std::get<0>(pick_data_tuple);
    edit_state_data_.world_point = std::get<1>(pick_data_tuple);
    if(picked_actor == nullptr){
        edit_state_data_.state = edit_state_;
        return edit_state_;
    }
    edit_state_data_.world_point = target_picker->GetPickPosition();
    if(axis_x_->hasSceneProp(&s, picked_actor)){
        edit_state_ = ES_AXIS_X_TRANSLATE;
        edit_state_data_.translate_axis_normal = axis_x_->direction();
        edit_state_data_.state = edit_state_;
        return edit_state_;
    }
    if(axis_y_->hasSceneProp(&s, picked_actor)){
        edit_state_ = ES_AXIS_Y_TRANSLATE;
        edit_state_data_.translate_axis_normal = axis_y_->direction();
        edit_state_data_.state = edit_state_;
        return edit_state_;
    }
    if(axis_z_->hasSceneProp(&s, picked_actor)){
        edit_state_ = ES_AXIS_Z_TRANSLATE;
        edit_state_data_.translate_axis_normal = axis_z_->direction();
        edit_state_data_.state = edit_state_;
        return edit_state_;
    }
    if(torus_x_->hasSceneProp(&s, picked_actor)){
        edit_state_ = ES_AXIS_X_ROTATE;
        edit_state_data_.rotate_axis_normal = torus_x_->getNormal();
        edit_state_data_.state = edit_state_;
        return edit_state_;
    }
    if(torus_y_->hasSceneProp(&s, picked_actor)){
        edit_state_ = ES_AXIS_Y_ROTATE;
        edit_state_data_.rotate_axis_normal = torus_y_->getNormal();
        edit_state_data_.state = edit_state_;
        return edit_state_;
    }
    if(torus_z_->hasSceneProp(&s, picked_actor)){
        edit_state_ = ES_AXIS_Z_ROTATE;
        edit_state_data_.rotate_axis_normal = torus_z_->getNormal();
        edit_state_data_.state = edit_state_;
        return edit_state_;
    }
    edit_state_data_.state = edit_state_;
    return edit_state_;
}

Vec3 PointMoveToolSprite::getAxisXDirection() const
{
    return axis_x_->direction();
}

Vec3 PointMoveToolSprite::getAxisYDirection() const
{
    return axis_y_->direction();
}

Vec3 PointMoveToolSprite::getAxisZDirection() const
{
    return axis_z_->direction();
}

NAMESPACE_END