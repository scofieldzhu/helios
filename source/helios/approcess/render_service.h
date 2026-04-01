/******************************************************** 
* author: scofieldzhu
* time:2026/2/6
*******************************************************/
#ifndef __render_service_h__
#define __render_service_h__

#include "helios/core/helios_core_typedef.h"

class vtkRenderWindowInteractor;

HELIOS_NAMESPACE_BEGIN

class RenderService
{
public:
	virtual void addScene(helios::Scene& s) = 0;
	virtual void requestRenderSceneViews() = 0;
	virtual void removeScene(helios::Scene& s) = 0;
	virtual void removeAllScenes() = 0;
	virtual vtkRenderWindowInteractor* getInteractor() = 0;
	virtual ~RenderService() = default;
};

NAMESPACE_END

#endif
