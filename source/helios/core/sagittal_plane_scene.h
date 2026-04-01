/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/12/17
*******************************************************/
#ifndef __sagittal_plane_scene_h__
#define __sagittal_plane_scene_h__

#include "plane_scene.h"

HELIOS_NAMESPACE_BEGIN

class HELIOS_CORE_API SagittalPlaneScene : public PlaneScene
{
	SCENE_DECL(SagittalPlaneScene, PlaneScene)
public:
	SagittalPlaneScene(const std::string_view& name);
	~SagittalPlaneScene();

protected:
	void handleStartRenderEvent() override;
};

NAMESPACE_END

#endif