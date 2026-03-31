/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/7/1
*******************************************************/
#ifndef __action_event_h__
#define __action_event_h__

#include "mirfak/core/mirfak_core_export.h"

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_CORE_API ActionEvent
{
public:
    /**
     * All the action events are defined here.
     */
    enum EventIds
    {
        NoEvent = 0,
        Select,
        EndSelect,
        Delete,
        Translate,
        EndTranslate,
        Scale,
        EndScale,
        Resize,
        EndResize,
        Rotate,
        EndRotate,
        Move,
        SizeHandles,
        AddPoint,
        AddFinalPoint,
        Completed,
        TimedOut,
        ModifyEvent,
        Reset,
        Up,
        Down,
        Left,
        Right,
        Select3D,
        EndSelect3D,
        Move3D,
        AddPoint3D,
        AddFinalPoint3D
    };
    /**
     * Convenience methods for translating between event names and event ids.
     */
    static const char* GetStringFromEventId(unsigned long event);
    static unsigned long GetEventIdFromString(const char* event);
    ActionEvent(const ActionEvent&) = delete;
    void operator=(const ActionEvent&) = delete;
};

NAMESPACE_END

#endif
