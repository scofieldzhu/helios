/******************************************************** 
* author: scofieldzhu
* time:2025/12/11
*******************************************************/
#ifndef __plane_scene_h__
#define __plane_scene_h__

#include "scene.h"

HELIOS_NAMESPACE_BEGIN

class HELIOS_CORE_API PlaneScene : public Scene
{
    SCENE_DECL(PlaneScene, Scene)
public:
    void setPlane(SlicePlaneSprite* plane);
    SlicePlaneSprite* plane(){ return plane_; }
    void fitViewToPlane();
    PlaneScene(const std::string_view& name);
    ~PlaneScene();

protected:
    void addToRenderWidget(RenderWidget& sw) override;
    void removeFromRenderWidget() override;
    virtual void handleStartRenderEvent();    
    SlicePlaneSprite* plane_ = nullptr;

private:
    static void StartRenderEventCallback(vtkObject* caller, unsigned long id, void* client_data, void*);    
    unsigned long start_event_tag_ = -1;
};

NAMESPACE_END

#endif