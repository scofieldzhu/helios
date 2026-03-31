/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/2/6
*******************************************************/
#ifndef __render_service_h__
#define __render_service_h__

#include "mirfak/core/mirfak_core_typedef.h"

class vtkRenderWindowInteractor;

MIRFAK_NAMESPACE_BEGIN

class RenderService
{
public:
	virtual void addScene(mirfak::Scene& s) = 0;
	virtual void requestRenderSceneViews() = 0;
	virtual void removeScene(mirfak::Scene& s) = 0;
	virtual void removeAllScenes() = 0;
	virtual vtkRenderWindowInteractor* getInteractor() = 0;
	virtual ~RenderService() = default;
};

NAMESPACE_END

#endif
