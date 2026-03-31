/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/1/25
*******************************************************/
#ifndef __signal_hpp__
#define __signal_hpp__

#include "mirfak/mirfak_nsp.h"
#include <map>
#include <mutex>
#include <atomic>
#include <functional>

MIRFAK_NAMESPACE_BEGIN

template <class ... Ts>
class Signal
{
public:
    using SlotIdType = long long;
    using SlotType = std::function<void(Ts...)>;

    auto bind(SlotType s){
        std::lock_guard guard_this(map_mutex_);
        auto next_id = genUniqueId();
        slot_map_[next_id] = s;
        return next_id;
    }

    auto clear(){
        slot_map_.clear();
    }

    auto unbind(SlotIdType id){
        std::lock_guard guard_this(map_mutex_);
        return slot_map_.erase(id);
    }

    template <class... ArgsT>
    void invoke(ArgsT&&... args){
        std::lock_guard guard_this(map_mutex_);
        auto slots_copy = slot_map_; //avoid removing operation occur inside slot function!
        for(const auto& kv : slots_copy){
            (kv.second)(std::forward<ArgsT>(args) ... );
        }
    }

private:
    SlotIdType genUniqueId()const{
        static std::atomic<SlotIdType> gGlobalCounter{0};  
        return gGlobalCounter.fetch_add(1, std::memory_order_relaxed);  
    }
    std::map<SlotIdType, SlotType> slot_map_;
    std::mutex map_mutex_;
};

NAMESPACE_END

#endif