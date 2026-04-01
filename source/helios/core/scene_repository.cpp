/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/2/3
*******************************************************/
#include "scene_repository.h"
#include <vtkRenderer.h>
#include "scene.h"

HELIOS_NAMESPACE_BEGIN

SceneRepository::SceneRepository()
{

}

void SceneRepository::addScene(Scene& s)
{
	managed_scenes_.push_back(&s);
}

void SceneRepository::removeScene(Scene& s)
{
	managed_scenes_.erase(std::find(managed_scenes_.begin(), managed_scenes_.end(), &s));
}

Scene* SceneRepository::getScene(CheckFuncType check_func) const
{
	auto it = std::find_if(managed_scenes_.begin(), managed_scenes_.end(), check_func);	
	return it == managed_scenes_.end() ? nullptr : *it;
}

Scene* SceneRepository::getScene(const std::string_view& name) const
{
	auto it = std::find_if(
		managed_scenes_.begin(),
		managed_scenes_.end(),
		[&name](auto s){
			return s->name() == name;
		}
	);	
	return it != managed_scenes_.end() ? (*it) : nullptr;
}

Scene* SceneRepository::getScene(int id) const
{
	return id < managed_scenes_.size() && id >= 0 ? managed_scenes_[id] : nullptr;
}

Scene* SceneRepository::getScene(vtkRenderer* ren) const
{
	for(auto s : managed_scenes_){
		if(s->getRenderer() == ren){
			return s;
		}
	}
	return nullptr;
}

bool SceneRepository::existScene(const Scene& s) const
{
	for(auto ms : managed_scenes_){
		if(ms == &s){
			return true;
		}
	}
	return false;
}

Scene* SceneRepository::getAxialScene() const
{
	return getScene(Axial);
}

Scene* SceneRepository::getCoronalScene() const
{
	return getScene(Coronal);
}

Scene* SceneRepository::getSagittalScene() const
{
	return getScene(Sagittal);
}

Scene* SceneRepository::getOrthoScene() const
{
	return getScene(Ortho);
}

void SceneRepository::popBack()
{
	if(!managed_scenes_.empty()){
		managed_scenes_.pop_back();
	}	
}

Scene* SceneRepository::back() const
{
	return managed_scenes_.empty() ? nullptr : managed_scenes_.back();
}

NAMESPACE_END