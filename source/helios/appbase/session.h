/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/15
*******************************************************/
#ifndef __session_h__
#define __session_h__

#include <vector>
#include <memory>
#include "helios/appbase/helios_appbase_export.h"
#include "helios/appbase/session_id.h"
#include "helios/appbase/session_context.h"

HELIOS_NAMESPACE_BEGIN

enum class SessionState
{
    kCreated,
    kRegistered,
    kEntered,
    kActive,
    kInactive,
    kLeaving,
    kUnregistered
};

class HELIOS_APPBASE_API Session : public MObject
{
public:
    using SessionUPtr = std::unique_ptr<Session>;
    void setContext(std::unique_ptr<SessionContext> ctx);
    SessionContext* context()const { return context_.get(); }
    template <class LC>
    LC* localContext()const{ return LC::SafeDownCast(context_.get()); }
    SessionId id()const{ return id_; }
    SessionState state()const{ return state_; }
    Session* parent()const{ return parent_; }
    const auto& children()const{ return children_; }
    bool hasChild()const{ return !children_.empty(); }
    Session* getActiveChild()const;
    Session* getFirstChild()const;
    template <class S>
    S* getConcreteFirstChild()const{
        static_assert(std::is_base_of_v<Session, S>);
        return S::SafeDownCast(getFirstChild());
    }    
    Session& operator=(const Session&) = delete;
    Session(const Session&) = delete;
    virtual ~Session();

protected:    
    virtual bool onEnter();
    virtual bool onActivate();
    virtual void onDeactivate();
    virtual void onLeave();
    virtual void onDead();
    explicit Session(Session* parent);    

private:
    friend class SessionManager;
    void removeChild(Session& s);
    void attachChild(SessionUPtr child);    
    SessionId id_;
    SessionState state_ = SessionState::kCreated;
    Session* parent_ = nullptr;
    std::vector<SessionUPtr> children_;
    std::unique_ptr<SessionContext> context_;    
};

NAMESPACE_END

#define SESSION_DECL(TheClass, SuperClass) MOBJECT_DECL(TheClass, SuperClass, helios::Session)

#endif