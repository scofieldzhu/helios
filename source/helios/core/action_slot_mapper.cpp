/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/7/1
*******************************************************/
#include "action_slot_mapper.h"
#include "event_translator.h"

HELIOS_NAMESPACE_BEGIN

ActionSlotMapper::ActionSlotMapper()
{
}

ActionSlotMapper::~ActionSlotMapper()
{
}

void ActionSlotMapper::setEventTranslator(EventTranslator* translator)
{
    event_translator_ = translator;
}

void ActionSlotMapper::setActionSlot(unsigned long vtk_event, unsigned long action_event, Editor* w, SlotType f)
{
    event_translator_->setTranslation(vtk_event, action_event);
    setActionSlot(action_event, w, f);
}

void ActionSlotMapper::setActionSlot(unsigned long action_event, Editor* w, SlotType f)
{
    editor_slot_map_[action_event] = EditorSlot(w, f);
}

void ActionSlotMapper::setActionSlot(unsigned long vtk_event, int modifiers, char key_code, int repeat_cnt, const char* key_sym, unsigned long action_event, Editor* w, SlotType f)
{
    event_translator_->setTranslation(vtk_event, modifiers, key_code, repeat_cnt, key_sym, action_event);
    setActionSlot(action_event, w, f);
}

void ActionSlotMapper::setActionSlot(unsigned long vtk_event, vtkEventData* event_data, unsigned long action_event, Editor* w, SlotType f)
{
    event_translator_->setTranslation(vtk_event, event_data, action_event);
    setActionSlot(action_event, w, f);
}

void ActionSlotMapper::invoke(unsigned long action_event)
{
    if(editor_slot_map_.contains(action_event)){
        auto& s = editor_slot_map_[action_event];
        (s.slot)(s.editor);
    }
}

NAMESPACE_END