/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/26
*******************************************************/
#include "process_view_model.h"
#include "process_session.h"

HELIOS_NAMESPACE_BEGIN

ProcessViewModel::ProcessViewModel(QObject* p, ProcessSession& s)
	:QObject(p),
	process_session_(s)
{

}

ProcessViewModel::~ProcessViewModel()
{

}

helios::ProcessIIDType ProcessViewModel::processIID () const
{
	return process_session_.processIID();
}

helios::ProcessSessionContext* ProcessViewModel::processSessionContext() const
{
	return process_session_.localContext<ProcessSessionContext>();
}

void ProcessViewModel::requestRenderSceneViews()
{
	processSessionContext()->requestRenderSceneViews();
}

NAMESPACE_END