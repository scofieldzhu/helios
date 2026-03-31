/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/1/26
*******************************************************/
#include "sprite.h"
#include <algorithm>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCellPicker.h>
#include <vtkMapper.h>
#include <vtkPoints.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkFollower.h>
#include <vtkProperty.h>
#include <vtkInformation.h>
#include <vtkInformationStringKey.h>
#include <vtkInformationDoubleVectorKey.h>
#include <vtkProp3DCollection.h>
#include <vtkTransform.h>
#include <vtkPickingManager.h>
#include <vtkHardwareSelector.h>
#include "scene.h"
#include "render_widget.h"
#include "mirfak/basic/sys_util.h"
#include "mirfak/basic/log_service.h"

MIRFAK_NAMESPACE_BEGIN

namespace
{
    void MergeActorCollection(vtkActorCollection* source, vtkActorCollection* target)
    {
        if(source == nullptr || target == nullptr || !source->GetNumberOfItems()){
            return;
        }
        source->InitTraversal();
        while(auto a = source->GetNextActor()){
            target->AddItem(a);
        }
    }

    vtkInformationDoubleVectorKey* PROP_COLOR_LABEL()
    {
        static vtkInformationDoubleVectorKey* key = vtkInformationDoubleVectorKey::MakeKey("PROP_COLOR_LABEL", "Sprite");
        return key;
    } 

    vtkInformationStringKey* PROP_NAME_LABEL()
    {
        static vtkInformationStringKey* key = vtkInformationStringKey::MakeKey("PROP_NAME_LABEL", "Sprite");
        return key;
    }

    using UniformPropValueSetter = std::function<void(vtkProp*)>;
    void SetUniformProperyValue(const std::map<Scene*, vtkPropCollection*>& scene_props, const Scene* scene, UniformPropValueSetter f)
    {
        for(const auto& kv : scene_props){
            if(scene && scene != kv.first){
                continue;
            }
            auto prop_list = kv.second;
            prop_list->InitTraversal();
            auto prop = prop_list->GetNextProp();
            while(prop){
                f(prop);
                prop = prop_list->GetNextProp();
            }
        }
    }

    UniformPropValueSetter MakeVisibilitySetter(unsigned int visibility)
    {
        return [visibility](vtkProp* prop){
            prop->SetVisibility(visibility);
        };
    }

    void SetUniformVisibility(const std::map<Scene*, vtkPropCollection*>& scene_props, const Scene* scene, unsigned int visibility)
    {        
        SetUniformProperyValue(scene_props, scene, MakeVisibilitySetter(visibility));
    }

    template <class ActorType>
    void SetActorColorImpl_T(ActorType* actor, const Color& clr)
    {
        assert(actor);
        actor->GetPropertyKeys()->Set(PROP_COLOR_LABEL(), clr.redR(), clr.greenR(), clr.blueR());
        actor->GetProperty()->SetColor(clr.toValuesR().data());
    }

    UniformPropValueSetter MakeColorSetter(const Color& clr)
    {
        return [&clr](vtkProp* prop){
            auto prop_actor = vtkActor::SafeDownCast(prop);
            if(prop_actor){
                SetActorColorImpl_T(prop_actor, clr);
                return;
            }
            auto prop_actor2d = vtkActor2D::SafeDownCast(prop);
            if(prop_actor2d){
                SetActorColorImpl_T(prop_actor2d, clr);
                return;
            }
        };
    }

    UniformPropValueSetter MakeHighlightColorSetter(const Color& clr)
    {
        return [&clr](vtkProp* prop){
            auto prop_actor = vtkActor::SafeDownCast(prop);
            if(prop_actor){
                prop_actor->GetProperty()->SetColor(clr.toValuesR().data());
                return;
            }
            auto prop_actor2d = vtkActor2D::SafeDownCast(prop);
            if(prop_actor2d){
                prop_actor2d->GetProperty()->SetColor(clr.toValuesR().data());
                return;
            }
        };
    }

    template <class ActorType>
    void RestoreActorColorImpl_T(ActorType* actor)
    {
        assert(actor);
        auto prop_keys = actor->GetPropertyKeys();
        if(prop_keys){
            double clr_data[3] = {0.0};
            prop_keys->Get(PROP_COLOR_LABEL(), clr_data);
            actor->GetProperty()->SetColor(clr_data[0], clr_data[1], clr_data[2]);
        }
    }

    UniformPropValueSetter MakeUnhighlightColorSetter()
    {
        return [](vtkProp* prop){
            auto prop_actor = vtkActor::SafeDownCast(prop);
            if(prop_actor){
                RestoreActorColorImpl_T(prop_actor);
                return;
            }
            auto prop_actor2d = vtkActor2D::SafeDownCast(prop);
            if(prop_actor2d){
                RestoreActorColorImpl_T(prop_actor2d);
                return;
            }
        };
    }

    void SetUniformColor(const std::map<Scene*, vtkPropCollection*>& scene_props, const Scene* scene, const Color& clr)
    {
        SetUniformProperyValue(scene_props, scene, MakeColorSetter(clr));
    }

    UniformPropValueSetter MakeOpacitySetter(double op)
    {
        return [op](vtkProp* prop){
            auto prop_actor = vtkActor::SafeDownCast(prop);
            if(prop_actor){
                prop_actor->GetProperty()->SetOpacity(op);
                return;
            }
            auto prop_actor2d = vtkActor2D::SafeDownCast(prop);
            if(prop_actor2d){
                prop_actor2d->GetProperty()->SetOpacity(op);
            }
        };
    }

    void SetUniformOpacity(const std::map<Scene*, vtkPropCollection*>& scene_props, const Scene* scene, double op)
    {
        SetUniformProperyValue(scene_props, scene, MakeOpacitySetter(op));
    }
}

Sprite::Sprite()
    :object_uid_(GenUuidString()),
    modifier_(vtkSmartPointer<vtkObject>::New()),
    render_timestamp_(vtkSmartPointer<vtkObject>::New()),
    transform_(vtkSmartPointer<vtkTransform>::New())
{
    transform_->Identity();
}

Sprite::~Sprite()
{
    for(auto& kv : scene_props_dict_){
        auto s = kv.first;
        auto props = kv.second;
        props->InitTraversal();
        auto p = props->GetNextProp();
        while(p){
            s->getRenderer()->RemoveViewProp(p);
            p = props->GetNextProp();
        };
        s->removeSprite(*this);
    }
}

std::string Sprite::GetPropName(vtkProp& prop)
{
    return prop.GetPropertyKeys()->Get(PROP_NAME_LABEL());
}

void Sprite::modified()
{
	modifier_->Modified();
}

void Sprite::setName(const std::string &name)
{
    name_ = name;
}

void Sprite::render()
{
	if(!hasChangedSince(render_timestamp_->GetMTime())){
        return;
    }
    std::vector<RenderWidget*> modified_scene_widgets;
    for(auto s : getDisplaySceneList()){
        auto this_widget = s->sceneWidget();
        if(std::find(modified_scene_widgets.begin(), modified_scene_widgets.end(), this_widget) == modified_scene_widgets.end()){
            modified_scene_widgets.push_back(this_widget);
        }
    }
    for(auto sw : modified_scene_widgets){
        sw->render();
    }
    render_timestamp_->Modified();
}

// void Sprite::setPickable(bool p)
// {
//     pickable_ = p;
//     for(auto child : children_){
//         child->setPickable(p);
//     }
// }

void Sprite::highlight(const Color& clr)
{
    SetUniformProperyValue(scene_props_dict_, nullptr, MakeHighlightColorSetter(clr));
    // for(auto child : children_){
    //     child->highlight(clr);
    // }
    modified();
    highlight_state_changed.invoke(this, true);
}

void Sprite::unhighlight()
{
    SetUniformProperyValue(scene_props_dict_, nullptr, MakeUnhighlightColorSetter());
    // for(auto child : children_){
    //     child->unhighlight();
    // }
    modified();    
    highlight_state_changed.invoke(this, false);
}

void Sprite::setVisibility(int v)
{
    if(visibility_ == v){
        return;
    }
    visibility_ = v;
    SetUniformVisibility(scene_props_dict_, nullptr, v);
    for(auto child : children_){
        child->setVisibility(v);
    }
    modified();
    visible_state_changed.invoke(this, v);
}

void Sprite::setSceneVisibility(const Scene& s, int v)
{
    SetUniformVisibility(scene_props_dict_, &s, v);
    for(auto child : children_){
        child->setSceneVisibility(s, v);
    }
    modified();
}

int Sprite::getVisibility(const Scene& s) const
{
    if(visibility_.has_value()){
        return visibility_.value();
    }
    for(auto& p : scene_props_dict_){
        auto props = p.second;
        if(&s != p.first){
            continue;
        }
        if(props->GetNumberOfItems()){
            props->InitTraversal();
            return props->GetNextProp()->GetVisibility();
        }else{
            if(!children_.empty()){
                return children_[0]->getVisibility(s);
            }
            return 0;
        }
    }
    return 0;
}

void Sprite::setColor(const Color& color)
{      
    color_ = color;
    SetUniformColor(scene_props_dict_, nullptr, color);
    // for(auto child : children_){
    //     child->setColor(color);
    // }
    modified();
}

ColorOpt Sprite::getColor()const
{
    return color_;
}

void Sprite::setOpacity(double opacity)
{
    opacity_ = opacity;
    SetUniformOpacity(scene_props_dict_, nullptr, opacity);
    for(auto child : children_){
        child->setOpacity(opacity);
    }
    modified();
}

// std::optional<double> Sprite::getOpacity() const
// {
//     return opacity_;
// }

vtkActor *Sprite::getSceneActor(const std::string &name, const Scene *scene) const
{
    for(auto& kv : scene_props_dict_){
        if(scene != nullptr && kv.first != scene){
            continue;
        }
        auto collection = kv.second;
        collection->InitTraversal();
        auto prop_obj = collection->GetNextProp();
        while(prop_obj){
            if(GetPropName(*prop_obj) == name){
                return vtkActor::SafeDownCast(prop_obj);
            }
            prop_obj = collection->GetNextProp();
        }
    }
    return nullptr;
}

bool Sprite::hasSceneProp(const Scene* scene, vtkProp* p) const
{
    for(auto& kv : scene_props_dict_){
        if(scene != nullptr && kv.first != scene){
            continue;
        }
        auto collection = kv.second;
        collection->InitTraversal();
        auto prop_obj = collection->GetNextProp();
        while(prop_obj){
            if(prop_obj == p){
                return true;
            }
            prop_obj = collection->GetNextProp();
        }
    }
    return false;
}

vtkActor* Sprite::getFirstSceneActor(const Scene* scene) const
{
	for(auto& kv : scene_props_dict_){
		if(scene != nullptr && kv.first != scene){
			continue;
		}        
		auto collection = kv.second;
        if(collection->GetNumberOfItems()){
			collection->InitTraversal();
			return vtkActor::SafeDownCast(collection->GetNextProp ());
        }
		break;
	}
	return nullptr;
}

vtkSmartPointer<vtkActorCollection> Sprite::getSceneActors(const Scene* scene, bool is_contain_child ) const
{
    auto result_actors = vtkSmartPointer<vtkActorCollection>::New();
    for(auto& kv : scene_props_dict_){
        if(scene != nullptr && kv.first != scene){
            continue;
        }
        auto collection = kv.second;
        collection->InitTraversal();
        auto prop_obj = collection->GetNextProp();
        while(prop_obj){
            if(prop_obj->IsA("vtkActor")){
                result_actors->AddItem(vtkActor::SafeDownCast(prop_obj));
            }
            prop_obj = collection->GetNextProp();
        }        
    }
    if(is_contain_child){
        for(auto c : children_){
            auto child_actors = c->getSceneActors(scene, is_contain_child);
            MergeActorCollection(child_actors, result_actors);
        }        
    }
    return result_actors;
}

void Sprite::getSceneProps(const Scene* const_scene, vtkPropCollection* result_props) const
{
    auto scene = const_cast<Scene*>(const_scene);
    if(result_props == nullptr || !scene_props_dict_.contains(const_cast<Scene*>(scene))){
        return;        
    }
    auto cur_collection = scene_props_dict_.at(scene);
    cur_collection->InitTraversal();
    while(auto cur_prop = cur_collection->GetNextProp()){
        result_props->AddItem(cur_prop);
    }
}

bool Sprite::addToScene(Scene& scene)
{
    making_actor_scene_ = &scene;
    if(!source_created_){
        createSource(scene);
        source_created_ = true;
    }
    if(!scene_props_dict_.contains(&scene)){
        scene_props_dict_.insert({&scene, vtkPropCollection::New()});
    }
    makeActors(scene);         
    making_actor_scene_ = nullptr;
    for(auto& child : children_){
        child->addToScene(scene);
    }
    connect_state_changed.invoke(&scene, this, true);
    return true;
}

void Sprite::removeFromScene(Scene& scene)
{     
    if(!scene_props_dict_.contains(&scene)){
        SPDLOG_WARN("The scene object not connected to this sprite!");
        return;
    }
    auto prop_list = scene_props_dict_[&scene];
    prop_list->InitTraversal();
    auto current_prop = prop_list->GetNextProp();
    while(current_prop != nullptr){
        scene.getRenderer()->RemoveViewProp(current_prop);
        current_prop = prop_list->GetNextProp();
    }
    scene_props_dict_.erase(&scene);
    for(auto& child : children_){
        child->removeFromScene(scene);
    }
    connect_state_changed.invoke(&scene, this, false);
}

void Sprite::createSource(Scene& scene)
{
    //Let derived class do it!
}

void Sprite::makeActors(Scene& scene)
{
    //Let derived class do it!
}

void Sprite::setTransform(vtkTransform* t)
{
    if(t == nullptr){
        transform_->Identity();
    }else{
        //transform_ = t;
        transform_->DeepCopy(t);
    }
    // for(auto& child : children_){
    //     child->getTransform()->SetInput(transform_);
    // }
    TransformChanged.invoke(this);
    modified();
}

vtkTransform *Sprite::getTransform() const
{
    return transform_.Get();
}

bool Sprite::hasChangedSince(unsigned long since_mtime)const
{
    if(modifier_->GetMTime() > since_mtime || transform_->GetMTime() > since_mtime){
        return true;
    }
    for(auto child : children_){   
        if(child->hasChangedSince(since_mtime)){
            return true;
        }
    }
    return false;
}

bool Sprite::addChild(Sprite& child)
{
    if(hasChild(child)){
        return false;
    }
    children_.push_back(&child);
    child.master_ = this;
    if(child.getTransform() != transform_.Get()){
        child.getTransform()->SetInput(transform_);
    }        
    for(const auto& kv : scene_props_dict_){
        child.addToScene(*kv.first);
    }
    //setPickable(pickable_); //keep uniform pickable with children'
    modified();
    return true;
}

bool Sprite::hasChild(const Sprite& child)const
{
    return std::find(children_.begin(), children_.end(), &child) != children_.end();
}

void Sprite::removeChild(Sprite& child)
{
    children_.erase(std::remove(children_.begin(), children_.end(), &child), children_.end());    
    child.master_ = nullptr;
    for(const auto& kv : scene_props_dict_){
        child.removeFromScene(*kv.first);
    }
}

void Sprite::removeChildren()
{
    while(!children_.empty()){
        removeChild(*children_.back());
        children_.pop_back();
    }
}

void Sprite::connectScene(Scene& scene)
{    
    if(!checkConnective(scene)){
        if(addToScene(scene)){
            scene.addSprite(*this);
        }
    }  
}

void Sprite::connectScenes(const SceneList& scenes)
{
    for(auto s : scenes){
        connectScene(*s);
    }
}

void Sprite::disconnectScene(Scene& scene)
{
    if(scene.hasSprite(*this)){ 
        removeFromScene(scene);
        scene.removeSprite(*this);
    }
}

void Sprite::disconnectScenes(const SceneList* scenes)
{
    SceneList target_scenes = (scenes == nullptr) ? getDisplaySceneList() : *scenes;
    for(auto s : target_scenes){
        disconnectScene(*s);
    }
}

bool Sprite::checkConnective(const Scene& scene) const
{
    return scene.hasSprite(*this);
}

SceneList Sprite::getDisplaySceneList() const
{
    SceneList result_scenes;
    for(auto& kv : scene_props_dict_){
        result_scenes.push_back(kv.first);
    }
    return result_scenes;
}

bool Sprite::existsDisplayScene() const
{
    return !scene_props_dict_.empty();
}

PickInfoList Sprite::getPickList(vtkCellPicker* picker, Scene& scene, int x, int y)const
{
    if(!(getVisibility(scene) && picker)){
        return {};
    }
    PickInfoList pick_result;    
    do{
        if(!scene_props_dict_.contains(&scene) || picker->GetProp3Ds()->GetNumberOfItems() < 1){
            break;
        }
        vtkPoints* picked_pos_list = picker->GetPickedPositions();
        auto scene_prop_list = scene_props_dict_.at(&scene);
        scene_prop_list->InitTraversal();
        auto cur_prop = scene_prop_list->GetNextProp();
        while(cur_prop){
            if(!cur_prop->GetPickable()){
                cur_prop = scene_prop_list->GetNextProp();
                continue;
            }
            auto pos = picker->GetProp3Ds()->IsItemPresent(cur_prop);
            if(pos == 0 || vtkActor::SafeDownCast(cur_prop) == nullptr){
                cur_prop = scene_prop_list->GetNextProp();
                continue;
            }
            auto position = picked_pos_list->GetPoint(pos - 1);
            auto cur_actor = vtkActor::SafeDownCast(cur_prop);
            vtkDataSet* data_set = cur_actor->GetMapper()->GetInputAsDataSet();
            vtkDataArray* normals = data_set->GetPointData()->GetNormals();
            Vec3 normal;
            if(normals){
                vtkNew<vtkTransform> trans;
                trans->SetMatrix(cur_actor->GetMatrix());
                auto data_pos = trans->GetInverse()->TransformPoint(position);
                auto id = data_set->FindPoint(data_pos[0], data_pos[1], data_pos[2]);
                normal = trans->TransformNormal(normals->GetTuple3(id));
            }
            PickInfo info;
            info.prop = cur_prop;
            info.position = position;
            info.normal = normal;
            info.sprite = const_cast<Sprite*>(this);                
            info.scene = &scene;
            pick_result.push_back(std::move(info));
            cur_prop = scene_prop_list->GetNextProp();
        };
    }while(0);
    for(auto child : children_){
        PickInfoList child_pick_result = child->getPickList(picker, scene, x, y);
        std::copy(child_pick_result.begin(), child_pick_result.end(), std::back_inserter(pick_result));
    }
    return pick_result;
}

void Sprite::addSceneProp(Scene& scene, vtkProp* prop, const std::string& name)
{
    if(prop == nullptr){
        SPDLOG_WARN("Null scene or prop object passed!");
        return;
    }
    if(making_actor_scene_ != &scene){
        SPDLOG_WARN("Call addSceneProp interface only at making actors!\n");
        return;
    }
    auto prop_keys = prop->GetPropertyKeys();
    if(prop_keys == nullptr){
        prop->SetPropertyKeys(vtkInformation::New());
    }
    prop->GetPropertyKeys()->Set(PROP_NAME_LABEL(), name);
    prop->GetPropertyKeys()->Set(PROP_COLOR_LABEL(), 0, 0, 0);
    scene_props_dict_[&scene]->AddItem(prop);    
    scene.getRenderer()->AddViewProp(prop);
    auto follower = vtkFollower::SafeDownCast(prop);
    if(follower){
        follower->SetCamera(scene.getRenderer()->GetActiveCamera());
    }
    auto prop_3d = vtkProp3D::SafeDownCast(prop);
    if(prop_3d){
        prop_3d->SetUserTransform(transform_);
    }
    if(color_){
        MakeColorSetter(color_.value())(prop);
    }
    if(opacity_){
        MakeOpacitySetter(opacity_.value())(prop);
    }
    if(visibility_){
        MakeVisibilitySetter(visibility_.value())(prop);
    }    
}

void Sprite::removeSceneProp(Scene& scene, const std::string& name)
{
    if(!scene_props_dict_.contains(&scene)){
        return;
    }
    auto prop_collection = scene_props_dict_[&scene];
    prop_collection->InitTraversal();
    auto current_prop = prop_collection->GetNextProp();
    while(current_prop != nullptr){
        if(GetPropName(*current_prop) == name){            
            prop_collection->RemoveItem(current_prop);
            break;
        }
        current_prop = prop_collection->GetNextProp();
    };
}

void Sprite::removeSceneProp(Scene& scene, vtkProp* a)
{
    if(!scene_props_dict_.contains(&scene)){
        return;
    }
    auto prop_collection = scene_props_dict_[&scene];
    prop_collection->InitTraversal();
    auto current_prop = prop_collection->GetNextProp();
    while(current_prop != nullptr){
        if(current_prop == a){
            prop_collection->RemoveItem(current_prop);
            break;
        }
        current_prop = prop_collection->GetNextProp();
    };
}

// void Sprite::onPicked(PickInfo& info)
// {
//     pick_state_changed.invoke(this, true);
// }

// void Sprite::onUnPicked(PickInfo* next_pick_info)
// {
//     pick_state_changed.invoke(this, false);
// }

void Sprite::startEditing(const Point2 &pt)
{
    is_editing_ = true;
}

void Sprite::interacting(const Point2 &pt)
{
}

void Sprite::endEditing(const Point2 &pt)
{
    is_editing_ = false;
}

int Sprite::computeEditState(Scene& s, int x, int y, int modify)
{
    return 0;
}

int Sprite::getEditState()const
{
    return edit_state_;
}

const void* Sprite::getEditStateData() const
{
    return nullptr;
}

vtkPickingManager* Sprite::getPickingManager(vtkRenderWindow* rw)
{
    if(rw == nullptr || rw->GetInteractor() == nullptr){
        return nullptr;
    }
    return rw->GetInteractor()->GetPickingManager();
}

void Sprite::registerPicker(vtkRenderWindow* rw, vtkAbstractPropPicker* p)
{
    if(!isRegisteredPicker(rw, p)){
        getPickingManager(rw)->AddPicker(p);
        pick_rws_.push_back(rw);
    }
}

void Sprite::unRegisterPicker(vtkRenderWindow* rw, vtkAbstractPropPicker* p)
{
    if(isRegisteredPicker(rw, p)){
        getPickingManager(rw)->RemovePicker(p);        
        pick_rws_.erase(std::find(pick_rws_.begin(), pick_rws_.end(), rw), pick_rws_.end());
    }
}

bool Sprite::isRegisteredPicker(vtkRenderWindow *rw, vtkAbstractPropPicker *p)
{
    auto pm = getPickingManager(rw);
    if(pm && p){
        return std::find(pick_rws_.begin(), pick_rws_.end(), rw) != pick_rws_.end();
    }
    return false;
}

vtkAssemblyPath* Sprite::getAssemblyPath(double x, double y, double z, Scene* s, vtkAbstractPropPicker* picker)
{
	picker->Pick (x, y, z, s->getRenderer ());
	return picker->GetPath();
        
    auto rw = s->getRenderWindow();
    if(!isRegisteredPicker(rw, picker)){
        picker->Pick(x, y, z, s->getRenderer());
        return picker->GetPath();
    }
    return getPickingManager(rw)->GetAssemblyPath(x, y, 0., picker, s->getRenderer(), nullptr);
}

void Sprite::registerPickers()
{
    //NOTHING TODO:
}

void Sprite::unRegisterPickers()
{
    //NOTHING TODO:
}

Sprite::SpriteGroupType* Sprite::createComponentGroup(const std::string_view& name)
{
    if(!component_groups_.contains(name)){        
        component_groups_[name] = std::make_unique<SpriteGroupType>();
    }else{
        SPDLOG_WARN("Component group with name \"{}\" already exists!", name);
    }
    return component_groups_[name].get();
}

Sprite::SpriteGroupType* Sprite::getComponentGroup(const std::string_view &name)
{
    return component_groups_.contains(name) ? component_groups_[name].get() : nullptr;
}

const Sprite::SpriteGroupType* Sprite::getComponentGroup(const std::string_view &name)const
{
    if(!component_groups_.contains(name)){
        return nullptr;
    }
    return component_groups_.at(name).get();
}

void Sprite::removeComponentGroup(const std::string_view& name)
{
    component_groups_.erase(name);
}

void Sprite::clearComponentGroups()
{
    component_groups_.clear();
}

NAMESPACE_END
