/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/4/18
*******************************************************/
#include "session.h"
#include "helios/basic/log_service.h"

HELIOS_NAMESPACE_BEGIN

Session::Session(Session* p)
	:parent_(p)
{

}

Session::~Session()
{

}

bool Session::onEnter()
{
	return true;
}

bool Session::onActivate()
{
	return true;
}

void Session::onDeactivate()
{

}

void Session::onLeave()
{

}

void Session::onDead()
{

}

void Session::attachChild(SessionUPtr child)
{
	child->parent_ = this;
	children_.emplace_back(std::move(child));
}

void Session::setContext(std::unique_ptr<SessionContext> ctx)
{
	context_ = std::move(ctx);
}

Session* Session::getFirstChild() const
{
	return children_.empty() ? nullptr : children_.front().get();
}

void Session::removeChild(Session& s)
{
	auto it = std::find_if(
		children_.begin(),
		children_.end(),
		[&s](auto& c){
			return c.get() == &s;
		}
	);
	if(it != children_.end()){
		children_.erase(it);
	}
	s.parent_ = nullptr;	
}

Session* Session::getActiveChild() const
{
	for(auto& c : children_){
		if(c->state_ == SessionState::kActive){
			return c.get();
		}
	}
	return nullptr;
}

NAMESPACE_END
