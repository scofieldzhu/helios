/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2023)
*******************************************************/
#ifndef __DEFTIMESCHEDULER_H__
#define __DEFTIMESCHEDULER_H__

#include <vector>
#include <mutex>
#include "timescheduler.h"
#include "visual_export.h"

HELIOS_NAMESPACE_BEGIN

class VISUAL_API DefTimeScheduler : public TimeScheduler
{
public:
    ScheduleId scheduleOnce(ScheduleTime millsec, const ScheduleFunc& func) override;
    ScheduleId scheduleEvery(ScheduleTime millsec, const ScheduleFunc& func) override;
    void unSchedule(ScheduleId id) override;
    void unScheduleAll() override;
    void handleScheduling() override;
    DefTimeScheduler();
    virtual ~DefTimeScheduler();

protected:    
    void updateScheduledFuncs();
    struct ScheduleItem 
    {
        ScheduleId id;
        ScheduleFunc func;
        uint64_t nexttimeslot;
        ScheduleTime duration; 
        bool dirty;
    };
    std::vector<ScheduleItem*> schedulefuncs_;
    std::vector<ScheduleItem*> readyfuncs_;
	std::mutex* readylock_;
};

NAMESPACE_END
#endif
