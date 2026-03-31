/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/12/17
*******************************************************/
#ifndef __axial_plane_scene_h__
#define __axial_plane_scene_h__

#include "plane_scene.h"

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_CORE_API AxialPlaneScene : public PlaneScene
{
	SCENE_DECL(AxialPlaneScene, PlaneScene)
public:
	AxialPlaneScene(const std::string_view& name);
	~AxialPlaneScene();

protected:
	void handleStartRenderEvent() override;
};

NAMESPACE_END

#endif