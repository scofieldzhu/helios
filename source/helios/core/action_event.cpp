/******************************************************** 
* author: scofieldzhu
* time:2025/7/1
*******************************************************/
#include "action_event.h"
#include <cstring>

HELIOS_NAMESPACE_BEGIN

// this list should only contain the initial, contiguous
// set of events and should not include UserEvent
static const char* ActionEventStrings[] = {
  "NoEvent",
  "Select",
  "EndSelect",
  "Delete",
  "Translate",
  "EndTranslate",
  "Scale",
  "EndScale",
  "Resize",
  "EndResize",
  "Rotate",
  "EndRotate",
  "Move",
  "SizeHandles",
  "AddPoint",
  "AddFinalPoint",
  "Completed",
  "TimedOut",
  "ModifyEvent",
  "Reset",
  nullptr,
};

const char* ActionEvent::GetStringFromEventId(unsigned long event)
{
    static unsigned long event_num = 0;
    // find length of table
    if(!event_num){
        while(ActionEventStrings[event_num] != nullptr){
            event_num++;
        }
    }
    return event < event_num ? ActionEventStrings[event] : "NoEvent";
}

//----------------------------------------------------------------------
unsigned long ActionEvent::GetEventIdFromString(const char* event)
{
    for(unsigned long i = 0; ActionEventStrings[i] != nullptr; i++){
        if(!strcmp(ActionEventStrings[i], event)){
            return i;
        }
    }
    return ActionEvent::NoEvent;
}

NAMESPACE_END
