/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/22
*******************************************************/
#include "process_session.h"
#include "process_view_model.h"

MIRFAK_NAMESPACE_BEGIN

ProcessSession::ProcessSession(Session* parent)
	:Session(parent)
{

}

ProcessSession::~ProcessSession()
{

}

bool ProcessSession::onEnter()
{
	if(handleEnterAction()){
		auto pc = processContext();
		if(pc){
			pc->viewmodel->emit sessionEnter();
		}
		return true;
	}
	return false;
}

bool ProcessSession::onActivate()
{
	if(handleActivateAction()){
		auto pc = processContext();
		if(pc){
			pc->viewmodel->emit sessionActivate();
		}
		return true;
	}
	return false;
}

void ProcessSession::onDeactivate()
{
	handleDeactivateAction();

	auto pc = processContext();
	if(pc){
		pc->viewmodel->emit sessionDeactivate();
	}
}

void ProcessSession::onLeave()
{
	handleLeaveAction();

	auto pc = processContext();
	if(pc){
		pc->viewmodel->emit sessionLeave();
	}
}

void ProcessSession::onDead()
{
	handleDeadAction();

	auto pc = processContext();
	if(pc){
		pc->viewmodel->emit sessionDead();
	}
}

ProcessSessionContext* ProcessSession::processContext() const
{
	return ProcessSessionContext::SafeDownCast(context());
}

NAMESPACE_END
