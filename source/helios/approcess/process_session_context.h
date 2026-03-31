/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/26
*******************************************************/
#ifndef __process_session_context_h__
#define __process_session_context_h__

#include "mirfak/core/scene_repository.h"
#include "mirfak/appbase/session_context.h"
#include "mirfak/appbase/progress_control_view.h"
#include "mirfak/approcess/render_service.h"
#include "mirfak/approcess/mirfak_approcess_typedef.h"

class vtkImageData;

MIRFAK_NAMESPACE_BEGIN

class ProcessSessionContext : public SessionContext
{
	SESSION_CONTEXT_DECL(ProcessSessionContext, SessionContext)
public:
	void requestRenderSceneViews(){
		return render_service->requestRenderSceneViews();
	}
	SceneList getACSOScenes()const{
		auto scenes = getACSScenes();
		scenes.push_back(scene_repository->getOrthoScene());
		return scenes;
	}
	SceneList getACSScenes()const{
		return {
			scene_repository->getAxialScene(),
			scene_repository->getCoronalScene(),
			scene_repository->getSagittalScene()
		};
	}
	const DICOMMetaData* dicom_meta_data = nullptr;
	vtkImageData* volume_data = nullptr;
	OrthoPlanesSprite* orthoplanes = nullptr;
	SurfaceVolumeSprite* volume = nullptr;
	std::unique_ptr<ProcessViewModel> viewmodel = nullptr;
	const SceneRepository* scene_repository = nullptr;
	RenderService* render_service = nullptr;
	ProgressControlView* progress_ctrl_view = nullptr;
};

NAMESPACE_END

#endif
