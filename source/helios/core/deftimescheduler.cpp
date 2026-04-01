/*******************************************************
*******************************************************/
#include <iterator>

#include <algorithm>
#include "deftimescheduler.h"
#include "basicFunctions.h"

HELIOS_NAMESPACE_BEGIN

namespace {
    ScheduleId gNextId = 0;
}

DefTimeScheduler::DefTimeScheduler()
    :readylock_(new std::mutex())
{
    //if(!readylock_->create(false))
    //    slog_fatal(kernellogger) << "create ready lock failed! reason:" << readylock_->lastError().c_str() << std::endl;
}

DefTimeScheduler::~DefTimeScheduler()
{
    unScheduleAll();
    delete readylock_;
}

ScheduleId DefTimeScheduler::scheduleOnce(ScheduleTime millsec, const ScheduleFunc& func)
{
    //readylock_->wait(INFINITE);
    //readyfuncs_.push_back(new ScheduleItem({++gNextId, func, GetSysTickCount() + millsec, 0, false}));
    //readylock_->release();
    //return gNextId;
	readylock_->try_lock();
	readyfuncs_.push_back(new ScheduleItem({ ++gNextId, func, GetSysTickCount() + millsec, 0, false }));
	readylock_->unlock();
	return gNextId;
}

ScheduleId DefTimeScheduler::scheduleEvery(ScheduleTime millsec, const ScheduleFunc& func)
{
    //readylock_->wait(INFINITE);
    //readyfuncs_.push_back(new ScheduleItem({++gNextId, func, GetSysTickCount() + millsec, millsec, false}));
    //readylock_->release();
    //return gNextId;
	readylock_->try_lock();
	readyfuncs_.push_back(new ScheduleItem({ ++gNextId, func, GetSysTickCount() + millsec, millsec, false }));
	readylock_->unlock();
	return gNextId;
}

void DefTimeScheduler::unSchedule(ScheduleId id)
{
    if(id > gNextId)
        return;
    for(auto it : schedulefuncs_){
        if(it->id == id){
            it->dirty = true;
            break;
        }
    }        
}

void DefTimeScheduler::unScheduleAll()
{
    //readylock_->wait(INFINITE);
	readylock_->lock();
    for(auto it : schedulefuncs_)
        delete it;
    schedulefuncs_.clear();
    for(auto it : readyfuncs_)
        delete it;
    readyfuncs_.clear();
    //readylock_->release();
	readylock_->unlock();
}

void DefTimeScheduler::handleScheduling()
{
    uint64_t now = GetSysTickCount();    
    for(auto it = schedulefuncs_.begin(); it != schedulefuncs_.end(); ++it){
        if(now > (*it)->nexttimeslot){
            if((*it)->duration <= 0)
                (*it)->dirty = true;
            else
                (*it)->nexttimeslot = now + (*it)->duration;
            (*it)->func();
        }
    }
    updateScheduledFuncs();
}

void DefTimeScheduler::updateScheduledFuncs()
{
    if(readylock_->try_lock())
        return;
    auto remit = remove_if(schedulefuncs_.begin(), schedulefuncs_.end(), [](const ScheduleItem* item) {return item->dirty; });
    if(remit != schedulefuncs_.end())
        schedulefuncs_.erase(remit, schedulefuncs_.end());
    if(!readyfuncs_.empty()){
        if(schedulefuncs_.empty())
            schedulefuncs_ = readyfuncs_;
        else            
            std::copy_n(readyfuncs_.begin(), readyfuncs_.size(), std::back_inserter(schedulefuncs_));
        readyfuncs_.clear();
    }
    readylock_->unlock();
}
NAMESPACE_END