/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/7/1
*******************************************************/
#ifndef __action_slot_mapper_h__
#define __action_slot_mapper_h__

#include <map>
#include "mirfak/core/mirfak_core_export.h"

class vtkEventData;

MIRFAK_NAMESPACE_BEGIN

class EventTranslator;
class Editor;

class MIRFAK_CORE_API ActionSlotMapper
{
public:
    void setEventTranslator(EventTranslator* translator);
    auto eventTranslator(){ return event_translator_; }

    /**
   * Convenient typedef for working with callbacks.
   */
    using SlotType = void(*)(Editor*);

    /**
     * This class works with the class vtkWidgetEventTranslator to set up the
     * initial coorespondence between VTK events, widget events, and callbacks.
     * Different flavors of the SetCallbackMethod() are available depending on
     * what sort of modifiers are to be associated with a particular event.
     * Typically the widgets should use this method to set up their event
     * callbacks. If modifiers are not provided (i.e., the vtk_event is a
     * unsigned long eventId) then modifiers are ignored. Otherwise, a vtkEvent
     * instance is used to fully quality the events.
     */
    void setActionSlot(unsigned long vtk_event, unsigned long action_event, Editor* w, SlotType f);
    void setActionSlot(unsigned long vtk_event, int modifiers, char key_code, int repeat_cnt, const char* key_sym, unsigned long action_event, Editor* w, SlotType f);
    void setActionSlot(unsigned long vtk_event, vtkEventData* event_data, unsigned long action_event, Editor* w, SlotType f);

    /**
     * This method invokes the callback given a widget event. A non-zero value
     * is returned if the listed event is registered.
     */
    void invoke(unsigned long action_event);

    ActionSlotMapper(const ActionSlotMapper&) = delete;
    ActionSlotMapper& operator=(const ActionSlotMapper&) = delete;
    ActionSlotMapper();
    ~ActionSlotMapper();

private:
    /**
     * This method is used to assign a callback (implemented as a static class
     * method) to a particular widget event. This is an internal method used by
     * widgets to map widget events into invocations of class methods.
     */
    void setActionSlot(unsigned long action_event, Editor* w, SlotType f);

    EventTranslator* event_translator_;
    struct EditorSlot
    {
        Editor* editor = nullptr;
        SlotType slot = nullptr;
        EditorSlot()
        {}
        EditorSlot(Editor* e, SlotType s)
            :editor(e),
            slot(s)
        {}
    };
    std::map<unsigned long, EditorSlot> editor_slot_map_;
};

NAMESPACE_END

#endif