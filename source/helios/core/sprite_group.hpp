/*******************************************************
* author: scofieldzhu
* time:2025/9/10
*******************************************************/
#ifndef __sprite_group_hpp__
#define __sprite_group_hpp__

#include <memory>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <type_traits>
#include "helios/basic/signal.hpp"

HELIOS_NAMESPACE_BEGIN

template <class T>
T* GetPtr(T* p) noexcept {
    return p;
}

template <class P>
auto GetPtr(const P& p) noexcept -> decltype(p.get()) {
    return p.get();
}

template <
    class S, 
    class MemberType = std::unique_ptr<S>
>
class SpriteGroup
{
public:
    using SpriteType = S;
    using SpritePtr = SpriteType*;

    Signal<SpritePtr, SpritePtr> CurrentChanged;
    Signal<SpritePtr> Added;
    Signal<SpritePtr> ToRemoved;
    Signal<> Clear;

    void clear()
    {
        setCurrentSprite(nullptr);
        sprites_.clear();
        Clear.invoke();
    }

    void setCurrentSprite(SpritePtr s)
    {
        if(s && !contain(s)){
            return;
        }
        if(current_sprite_ != s){
            auto old = current_sprite_;
            current_sprite_ = s;
            CurrentChanged.invoke(current_sprite_, old);
        }        
    }

    bool contain(SpritePtr s)const
    {
        return s && 
            std::any_of(
                sprites_.begin(), 
                sprites_.end(), 
                [s](const auto& p){ return GetPtr(p) == s;}
            );
    }

    SpritePtr findByUID(const std::string& uid)const
    {
        auto it = std::find_if(
            sprites_.begin(), 
            sprites_.end(),
            [&uid](auto& p){
                return p->objectUID() == uid;
            }
        );
        return it != sprites_.end() ? (*it).get() : nullptr;
    }

    SpritePtr find(const std::string& name)const
    {
        auto it = std::find_if(
            sprites_.begin(), 
            sprites_.end(),
            [&name](auto& p){
                return p->name() == name;
            }
        );
        return it != sprites_.end() ? (*it).get() : nullptr;
    }

    void append(MemberType s)
    {
        if(s){
            auto ptr = GetPtr(s);
            sprites_.emplace_back(std::move(s));
            Added.invoke(ptr);
        }        
    }

    void remove(const std::string& name)
    {
        auto it = std::remove_if(
            sprites_.begin(), 
            sprites_.end(),
            [&name](auto& p){ return p->name() == name;}
        );
        if(it != sprites_.end()){
            auto p = GetPtr(*it);
            if(p == current_sprite_){
                setCurrentSprite(nullptr);
            }
            ToRemoved.invoke(p);
            sprites_.erase(it);            
        }
    }

    void remove(SpritePtr p)
    {
        auto it = std::remove_if(
            sprites_.begin(), 
            sprites_.end(),
            [p](const auto& it){ return GetPtr(*it) == p;}
        );
        if(it != sprites_.end()){
            auto p = GetPtr(*it);
            if(p == current_sprite_){
                setCurrentSprite(nullptr);
            }
            ToRemoved.invoke(p);
            sprites_.erase(it);            
        }
    }

    SpritePtr beginTraverse()
    {
        if(sprites_.empty()){
            return nullptr;
        }
        cursor_ = sprites_.begin();
        return  GetPtr(*cursor_);
    }

    SpritePtr next()
    {
        ++cursor_;
        return cursor_ != sprites_.end() ? GetPtr(*cursor_) : nullptr;
    }

    SpritePtr operator[](int index)const
    {
        if(index < 0 || index >= sprites_.size()){
            throw std::out_of_range("Bad subscript value!");
        }
        return GetPtr(sprites_[index]);
    }

    auto size()const{ return sprites_.size(); }

    auto empty()const{ return sprites_.empty(); }

    SpriteGroup() = default;
    ~SpriteGroup() = default;

private:
    using SpriteListType = std::vector<MemberType>;
    SpriteListType sprites_;
    SpriteListType::iterator cursor_;
    SpritePtr current_sprite_ = nullptr;
};

NAMESPACE_END

#endif