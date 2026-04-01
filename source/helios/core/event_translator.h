/******************************************************** 
* author: scofieldzhu
* time:2025/7/1
*******************************************************/
#ifndef __event_translator_h__
#define __event_translator_h__

#include "helios/core/helios_core_typedef.h"

class vtkEvent;
class vtkEventData;
class vtkCallbackCommand;
class vtkRenderWindowInteractor;

HELIOS_NAMESPACE_BEGIN

class EventTranslator
{
public:
    void setTranslation(unsigned long vtk_event, unsigned long action_event);
    void setTranslation(const char* vtk_event, const char* action_event);
    void setTranslation(unsigned long vtk_event, int modifier, char keyCode, int repeatCount, const char* keySym, unsigned long action_event);
    void setTranslation(vtkEvent* vtk_event, unsigned long widgetEvent);
    void setTranslation(unsigned long vtk_event, vtkEventData* edata, unsigned long action_event);

    /**
     * Translate a VTK event into a widget event. If no event mapping is found,
     * then the methods return vtkWidgetEvent::NoEvent or a nullptr string.
     */
    unsigned long getTranslation(unsigned long vtk_event);
    const char* getTranslation(const char* vtk_event);
    unsigned long getTranslation(unsigned long vtk_event, int modifier, char keyCode, int repeatCount, const char* keySym);
    unsigned long getTranslation(unsigned long vtk_event, vtkEventData* edata);
    unsigned long getTranslation(vtkEvent* vtk_event);

    /**
     * Remove translations for a binding.
     * Returns the number of translations removed.
     */
    int removeTranslation(unsigned long vtk_event, int modifier, char keyCode, int repeatCount, const char* keySym);
    int removeTranslation(vtkEvent* e);
    int removeTranslation(vtkEventData* e);
    int removeTranslation(unsigned long vtk_event);
    int removeTranslation(const char* vtk_event);

    /**
     * Clear all events from the translator (i.e., no events will be
     * translated).
     */
    void clearEvents();

    /**
     * Add the events in the current translation table to the interactor.
     */
    //void addEventsToParent(vtkAbstractWidget*, vtkCallbackCommand*, float priority);
    void addEventsToInteractor(vtkRenderWindowInteractor*, vtkCallbackCommand*, float priority);

    EventTranslator(const EventTranslator&) = delete;
    bool operator=(const EventTranslator&) = delete;
    EventTranslator();
    ~EventTranslator();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

NAMESPACE_END

#endif