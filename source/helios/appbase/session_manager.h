/******************************************************** 
* author: scofieldzhu
* time:2026/1/15
*******************************************************/
#ifndef __session_manager_h__
#define __session_manager_h__

#include <unordered_map>
#include <atomic>
#include "helios/appbase/session.h"

HELIOS_NAMESPACE_BEGIN

class HELIOS_APPBASE_API SessionManager
{
public:
    template<typename T, typename... Args>
    T* createRootSession(Args&&... args);
	Session* rootSession(){ return root_.get(); }
	template<class T>
	T* concreteRootSession(){
		static_assert(std::is_base_of_v<Session, T>);
		return T::SafeDownCast(root_.get());
	}
    template<typename T, typename... Args>
    T* createChildSession(Session* parent, Args&&... args);
	template<typename T>
	void takeChildSession(std::unique_ptr<T> child);
    virtual void enter(Session* s);
	virtual void activate(Session* s);
	virtual void deactivate(Session* s);
	virtual void leave(Session* s);
	virtual void destroySession(Session* s);
    Session* findById(SessionId id) const;
    template<typename T>
    T* findFirstOfType(Session* scope = nullptr) const;
    const std::vector<Session*>& activePath() const;

private:
    void assignId(Session* s);
    void unregisterId(Session* s);
    void deactivateActiveSibling(Session* s);
    void popActivePath(Session* s);
    template<typename T>
    T* findTypeRecursive(Session* s) const;
	std::unique_ptr<Session> root_;
    std::vector<Session*> active_path_; 
    std::unordered_map<std::string, Session*> index_dict_;
};

template<typename T, typename...Args>
T* SessionManager::createRootSession(Args&&... args)
{
	if(root_){ //avoid more roots born!
		return nullptr;
	}
	static_assert(std::is_base_of_v<Session, T>);
	auto session = std::make_unique<T>(nullptr, std::forward<Args> (args)...);
	assignId(session.get());
	T* raw = session.get();
	root_ = std::move(session);
	return raw;
}

template<typename T, typename...Args>
T* SessionManager::createChildSession(Session* parent, Args&&... args)
{
	if(parent == nullptr){
		return nullptr;
	}
	static_assert(std::is_base_of_v<Session, T>);
	auto session = std::make_unique<T>(parent, std::forward<Args> (args)...);
	assignId(session.get());	
	T* raw = session.get();
	parent->attachChild(std::move(session));
	return raw;
}

template<typename T>
void SessionManager::takeChildSession(std::unique_ptr<T> child)
{
	static_assert(std::is_base_of_v<Session, T>);
	if(child.get() == nullptr || child->parent() == nullptr){
		return;
	}
	assignId(child.get());	
	child->parent()->attachChild(std::move(child));
}

template<typename T>
T* SessionManager::findFirstOfType(Session* scope) const
{
	static_assert(std::is_base_of_v<Session, T>);
	if(scope){
		return findTypeRecursive<T>(scope);
	}
	if(auto r = findTypeRecursive<T>(root_.get())){
		return r;
	}
	return nullptr;
}

template<typename T>
T* SessionManager::findTypeRecursive(Session* s) const
{
	if(auto t = T::SafeDownCast(s)){
		return t;
	}
	for(auto& child : s->children_){
		if(auto r = findTypeRecursive<T>(child.get())){
			return r;
		}
	}
	return nullptr;
}

NAMESPACE_END

#endif
