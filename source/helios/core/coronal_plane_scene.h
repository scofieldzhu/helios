/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/12/17
*******************************************************/
#ifndef __coronal_plane_scene_h__
#define __coronal_plane_scene_h__

#include "plane_scene.h"

HELIOS_NAMESPACE_BEGIN

class HELIOS_CORE_API CoronalPlaneScene : public PlaneScene
{
	SCENE_DECL(CoronalPlaneScene, PlaneScene)
public:
	CoronalPlaneScene(const std::string_view& name);
	~CoronalPlaneScene();

protected:
	void handleStartRenderEvent() override;
};

NAMESPACE_END

#endif