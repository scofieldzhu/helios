/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/30
*******************************************************/
#include "scene_layout_manager.h"
#include "helios/core/scene.h"
#include "helios/core/scene_repository.h"

HELIOS_NAMESPACE_BEGIN

namespace{
	void HideScene(Scene* s)
	{
		s->setViewport(0.0, 0.0, 0.0001, 0.0001);
	}
}

SceneLayoutManager::SceneLayoutManager(QObject* parent /*= nullptr*/)
	:QObject(parent)
{

}

void SceneLayoutManager::onMinmaxSceneView(const QString& name, bool maxmized)
{
	if(maxmized){
		maximizeScene(name);
	}else{
		restoreScene();
	}	
}

void SceneLayoutManager::applyLayout(SceneLayout* layout)const
{
	if(scene_repos_ == nullptr){
		return;
	}
	for(auto i = 0; i < scene_repos_->getNumberOfScenes(); ++i){
		auto s = scene_repos_->getScene(i);
		if(!layout->scene_vp_dict_.contains(s->name())){
			HideScene(s);
			continue;
		}
		auto new_vp = layout->scene_vp_dict_[s->name()];
		s->setViewport(new_vp[0], new_vp[1], new_vp[2], new_vp[3]);
		s->resetView();
	}
}

void SceneLayoutManager::maximizeScene(const QString& max_scene_name)
{
	auto active_layout = getLayout(active_layout_name_);
	if(active_layout && scene_repos_){
		auto border = active_layout->border_;
		for(auto i = 0; i < scene_repos_->getNumberOfScenes(); ++i){
			auto s = scene_repos_->getScene(i);
			if(s->name() == max_scene_name.toStdString()){
				s->setViewport(border, border, 1.0 - border, 1.0 - border);
				s->resetView();
			}else{
				HideScene(s);
			}
		}
		maximized_scene_name_ = max_scene_name;
	}		
}

void SceneLayoutManager::restoreScene()
{
	maximized_scene_name_.clear();
	updateActiveLayout();
}

void SceneLayoutManager::updateActiveLayout()
{
	if(!maximized_scene_name_.isEmpty()){
		maximizeScene(maximized_scene_name_);
	}else{
		auto active_layout = getLayout(active_layout_name_);
		if(active_layout){
			applyLayout(active_layout);
		}
	}
	emit activeLayoutUpdated();
}

void SceneLayoutManager::makeActiveLayout(const QString& layout_name)
{
	if(active_layout_name_ != layout_name){
		auto layout = getLayout(layout_name);
		if(layout){
			applyLayout(layout);
			active_layout_name_ = layout_name;
		}
	}
}

void SceneLayoutManager::addLayout(SceneLayoutUPtr layout)
{
	layouts_.emplace_back(std::move(layout));
}

SceneLayout* SceneLayoutManager::getLayout(const QString& name_val)const
{
	for(auto& layout : layouts_){
		if(layout->name() == name_val){
			return layout.get();
		}
	}
	return nullptr;
}

void SceneLayoutManager::setSceneRepository(const SceneRepository* s)
{
	scene_repos_ = s;
}

NAMESPACE_END
