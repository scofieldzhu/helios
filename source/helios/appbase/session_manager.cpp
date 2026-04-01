/******************************************************** 
* author: scofieldzhu
* time:2026/1/15
*******************************************************/
#include "session_manager.h"
//#include <QUuid>
#include "session.h"
#include "helios/basic/sys_util.h"
#include "helios/basic/log_service.h"

HELIOS_NAMESPACE_BEGIN

//std::string GenUuidString()
//{
//	return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
//}

void SessionManager::enter(Session* s)
{
	if(s && s->state_ == SessionState::kRegistered){
		if(s->onEnter()){
			s->state_ = SessionState::kEntered;
		}		
	}
}

void SessionManager::activate(Session* s)
{
	if(s && (s->state_ == SessionState::kEntered || s->state_ == SessionState::kInactive)){
		deactivateActiveSibling(s);
		if(s->onActivate()){
			s->state_ = SessionState::kActive;
			active_path_.push_back(s);
		}
	}	
}

void SessionManager::deactivate(Session* s)
{
	if(s && s->state_ == SessionState::kActive){
		s->onDeactivate();
		s->state_ = SessionState::kInactive;
		popActivePath(s);
	}	
}

void SessionManager::leave(Session* s)
{
	if(s == nullptr || s->state_ == SessionState::kLeaving || s->state_ == SessionState::kUnregistered){
		return;
	}
	if(s->state_ == SessionState::kActive){
		deactivate(s);
	}
	auto check_state_func = [](SessionState s){
		return s == SessionState::kActive || s == SessionState::kInactive || s == SessionState::kEntered; 
	};
	s->state_ = SessionState::kLeaving;
	for(auto& child : s->children_){
		if(check_state_func(child->state_)){
			leave(child.get());
		}
	}
	s->onLeave();
	unregisterId(s);
}

Session* SessionManager::findById(SessionId id) const
{
	auto it = index_dict_.find(id.uuid);
	return it == index_dict_.end() ? nullptr : it->second;
}

const std::vector<Session*>& SessionManager::activePath() const
{
	return active_path_;
}

void SessionManager::assignId(Session* s)
{
	SessionId new_id{GenUuidString()};
	s->id_ = new_id;
	index_dict_[new_id.uuid] = s;
	s->state_ = SessionState::kRegistered;
}

void SessionManager::unregisterId(Session* s)
{
	if(s){
		index_dict_.erase(s->id().uuid);
		s->state_ = SessionState::kUnregistered;
	}	
}

void SessionManager::deactivateActiveSibling(Session* s)
{
	if(!s->parent()){
		return;
	}
	for(auto& sibling : s->parent()->children()){
		if(sibling.get () != s &&
			sibling->state_ == SessionState::kActive){
			deactivate(sibling.get());
		}
	}
}

void SessionManager::popActivePath(Session* s)
{
	auto it = std::find(active_path_.begin(), active_path_.end(), s);
	if(it != active_path_.end()){
		active_path_.erase(it, active_path_.end());
	}
}

void SessionManager::destroySession(Session* s)
{
	if(s == nullptr){ 
		return;
	}
	leave(s);
	while(s->hasChild()){
		destroySession(s->getFirstChild());
	}
	if(auto p = s->parent()){
		s->onDead();
		p->removeChild(*s);
	}	
}

NAMESPACE_END