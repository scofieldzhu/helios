/******************************************************** 
* author: scofieldzhu
* time:2025/7/1
*******************************************************/
#include "event_translator.h"
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkEventData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <list>
#include <map>
#include "action_event.h"

HELIOS_NAMESPACE_BEGIN

struct EventItem
{
    vtkSmartPointer<vtkEvent> vtk_event;
    unsigned long action_event;
    vtkEventData* event_data = nullptr;
    bool has_data = false;

    EventItem() = delete;

    EventItem(vtkEvent* v_e, unsigned long a_e)
        :vtk_event(v_e),
        action_event(a_e)
    {}

    EventItem(vtkEventData* evt_data, unsigned long a_e)
        :event_data(evt_data),
        action_event(a_e),
        has_data(true)
    {
        event_data->Register(nullptr);
    }

    EventItem(const EventItem& v)
    {
        vtk_event = v.vtk_event;
        action_event = v.action_event;
        has_data = v.has_data;
        event_data = v.event_data;
        if(has_data && event_data){
            event_data->Register(nullptr);
        }
    }

    ~EventItem()
    {
        if(has_data && event_data){
            event_data->UnRegister(nullptr);
            event_data = nullptr;
        }
    }
};

// A list of events
struct EventList : public std::list<EventItem>
{
    unsigned long find(unsigned long vtk_event)
    {
        for(auto it = begin(); it != end(); ++it){
            if(vtk_event == it->vtk_event->GetEventId()){
                return it->action_event;
            }
        }
        return ActionEvent::NoEvent;
    }

    unsigned long find(vtkEvent* vtk_event)
    {
        for(auto it = begin(); it != end(); ++it){
            if(*vtk_event == it->vtk_event){
                return it->action_event;
            }
        }
        return ActionEvent::NoEvent;
    }

    unsigned long find(vtkEventData* edata)
    {
        for(auto it = begin(); it != end(); ++it){
            if(it->has_data && *edata == *it->event_data){
                return it->action_event;
            }
        }
        return ActionEvent::NoEvent;
    }

    // Remove a mapping
    int Remove(vtkEvent* vtk_event)
    {
        for(auto it = begin(); it != end(); ++it){
            if(*vtk_event == it->vtk_event){
                erase(it);
                return 1;
            }
        }
        return 0;
    }

    int Remove(vtkEventData* edata)
    {
        for(auto it = begin(); it != end(); ++it){
            if(it->has_data && *edata == *it->event_data){
                erase(it);
                return 1;
            }
        }
        return 0;
    }
};

// A STL map used to translate VTK events into lists of events. The reason
// that we have this list is because of the modifiers on the event. The
// VTK event id maps to the list, and then comparisons are done to
// determine which event matches.
class EventMap : public std::map<unsigned long, EventList>
{
};

struct EventTranslator::Impl
{
    // Map VTK events to action events
    EventMap* event_map;
    // Used for performance reasons to avoid object construction/deletion
    vtkEvent* temp_event;

    Impl()
    {
        event_map = new EventMap;
        temp_event = vtkEvent::New();
    }

    ~Impl()
    {
        delete event_map;
        temp_event->Delete();
    }
};


EventTranslator::EventTranslator()
    :impl_(std::make_unique<Impl>())
{
}

EventTranslator::~EventTranslator()
{
}

void EventTranslator::setTranslation(unsigned long vtk_event, unsigned long action_event)
{
    auto e = vtkSmartPointer<vtkEvent>::New();
    e->SetEventId(vtk_event); // default modifiers
    if(action_event != ActionEvent::NoEvent){
        (*impl_->event_map)[vtk_event].push_back(EventItem(e, action_event));
    }else{
        removeTranslation(e);
    }
}

void EventTranslator::setTranslation(const char* vtk_event, const char* action_event)
{
    setTranslation(vtkCommand::GetEventIdFromString(vtk_event), ActionEvent::GetEventIdFromString(action_event));
}

void EventTranslator::setTranslation(unsigned long vtk_event, int modifier, char keyCode, int repeatCount, const char* keySym, unsigned long action_event)
{
    vtkSmartPointer<vtkEvent> e = vtkSmartPointer<vtkEvent>::New();
    e->SetEventId(vtk_event);
    e->SetModifier(modifier);
    e->SetKeyCode(keyCode);
    e->SetRepeatCount(repeatCount);
    e->SetKeySym(keySym);
    if(action_event != ActionEvent::NoEvent){
        (*impl_->event_map)[vtk_event].push_back(EventItem(e, action_event));
    }else{
        removeTranslation(e);
    }
}

void EventTranslator::setTranslation(unsigned long vtk_event, vtkEventData* edata, unsigned long action_event)
{
    if(action_event != ActionEvent::NoEvent){
        (*impl_->event_map)[vtk_event].push_back(EventItem(edata, action_event));
    }else{
        removeTranslation(edata);
    }
}

void EventTranslator::setTranslation(vtkEvent* vtk_event, unsigned long action_event)
{
    if(action_event != ActionEvent::NoEvent){
        (*impl_->event_map)[vtk_event->GetEventId()].push_back(EventItem(vtk_event, action_event));
    }else{
        removeTranslation(vtk_event);
    }
}

unsigned long EventTranslator::getTranslation(unsigned long vtk_event)
{
    auto iter = impl_->event_map->find(vtk_event);
    if(iter != impl_->event_map->end()){
        EventList& elist = (*iter).second;
        return elist.find(vtk_event);
    }else{
        return ActionEvent::NoEvent;
    }
}

const char* EventTranslator::getTranslation(const char* vtk_event)
{
    return ActionEvent::GetStringFromEventId(getTranslation(vtkCommand::GetEventIdFromString(vtk_event)));
}

unsigned long EventTranslator::getTranslation(unsigned long vtk_event, int modifier, char key_code, int repeat_cnt, const char* key_sym)
{
    auto iter = impl_->event_map->find(vtk_event);
    if(iter != impl_->event_map->end()){
        impl_->temp_event->SetEventId(vtk_event);
        impl_->temp_event->SetModifier(modifier);
        impl_->temp_event->SetKeyCode(key_code);
        impl_->temp_event->SetRepeatCount(repeat_cnt);
        impl_->temp_event->SetKeySym(key_sym);
        EventList& e_list = (*iter).second;
        return e_list.find(impl_->temp_event);
    }
    return ActionEvent::NoEvent;
}

unsigned long EventTranslator::getTranslation(unsigned long, vtkEventData* evt_data)
{
    auto iter = impl_->event_map->find(evt_data->GetType());
    if(iter != impl_->event_map->end()){
        EventList& elist = (*iter).second;
        return elist.find(evt_data);
    }
    return ActionEvent::NoEvent;
}

unsigned long EventTranslator::getTranslation(vtkEvent* vtk_event)
{
    auto iter = impl_->event_map->find(vtk_event->GetEventId());
    if(iter != impl_->event_map->end()){
        EventList& elist = (*iter).second;
        return elist.find(vtk_event);
    }else{
        return ActionEvent::NoEvent;
    }
}

int EventTranslator::removeTranslation(unsigned long vtk_event, int modifier, char keyCode, int repeatCount, const char* keySym)
{
    auto e = vtkSmartPointer<vtkEvent>::New();
    e->SetEventId(vtk_event);
    e->SetModifier(modifier);
    e->SetKeyCode(keyCode);
    e->SetRepeatCount(repeatCount);
    e->SetKeySym(keySym);
    return removeTranslation(e);
}

int EventTranslator::removeTranslation(vtkEvent* e)
{
    auto iter = impl_->event_map->find(e->GetEventId());
    int num_to_removed = 0;
    if(iter == impl_->event_map->end()){
        return num_to_removed;
    }
    while(iter->second.Remove(e)){
        ++num_to_removed;
        iter = impl_->event_map->find(e->GetEventId());
        if(iter == impl_->event_map->end()){
            break;
        }
    }
    return num_to_removed;
}

int EventTranslator::removeTranslation(vtkEventData* edata)
{
    auto iter = impl_->event_map->find(edata->GetType());
    int num_to_removed = 0;
    if(iter != impl_->event_map->end()){
        while (iter->second.Remove(edata)){
            ++num_to_removed;
            iter = impl_->event_map->find(edata->GetType());
            if(iter == impl_->event_map->end()){
                break;
            }
        }
    }
    return num_to_removed;
}

int EventTranslator::removeTranslation(unsigned long vtk_event)
{
    auto e = vtkSmartPointer<vtkEvent>::New();
    e->SetEventId(vtk_event);
    return removeTranslation(e);
}

int EventTranslator::removeTranslation(const char* vtk_event)
{
    auto e = vtkSmartPointer<vtkEvent>::New();
    e->SetEventId(vtkCommand::GetEventIdFromString(vtk_event));
    return removeTranslation(e);
}

void EventTranslator::clearEvents()
{
    for(auto iter = impl_->event_map->begin(); iter != impl_->event_map->end(); ++iter){
        EventList& elist = (*iter).second;
        elist.clear();
    }
    impl_->event_map->clear();
}

void EventTranslator::addEventsToInteractor(vtkRenderWindowInteractor* i, vtkCallbackCommand* command, float priority)
{
    for(auto iter = impl_->event_map->begin(); iter != impl_->event_map->end(); ++iter){
        i->AddObserver((*iter).first, command, priority);
    }
}

// void VtkEventTranslator::addEventsToParent(
//   vtkAbstractWidget* w, vtkCallbackCommand* command, float priority)
// {
//   auto iter = EventMap->begin();
//   for (; iter != EventMap->end(); ++iter)
//   {
//     w->AddObserver((*iter).first, command, priority);
//   }
// }

NAMESPACE_END
