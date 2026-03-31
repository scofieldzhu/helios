/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/1/26
*******************************************************/
#include "process_view.h"
#include "process_view_model.h"

MIRFAK_NAMESPACE_BEGIN

ProcessView::ProcessView(QWidget* p)
	:QWidget(p)
{

}

ProcessView::~ProcessView()
{

}

void ProcessView::onSessionActivate()
{

}

void ProcessView::onSessionDeactivate()
{

}

void ProcessView::onSessionEnter()
{

}

void ProcessView::onSessionLeave()
{
	releaseActiveConnections();
}

void ProcessView::onSessionDead()
{
	setViewModel(nullptr);
}

void ProcessView::setViewModel(ProcessViewModel* vm)
{
	if(viewmodel_ == vm){
		return;
	}
	if(viewmodel_){
		viewmodel_->disconnect(this);
	}
	viewmodel_ = vm;
	if(viewmodel_){
		connect(vm, &ProcessViewModel::sessionActivate, this, &ProcessView::onSessionActivate);
		connect(vm, &ProcessViewModel::sessionDeactivate, this, &ProcessView::onSessionDeactivate);
		connect(vm, &ProcessViewModel::sessionEnter, this, &ProcessView::onSessionEnter);
		connect(vm, &ProcessViewModel::sessionLeave, this, &ProcessView::onSessionLeave);
		connect(vm, &ProcessViewModel::sessionDead, this, &ProcessView::onSessionDead);
	}
	onViewModelSet();
}

void ProcessView::setTitle(const QString& title)
{
	title_ = title;
}

void ProcessView::onViewModelSet()
{

}

void ProcessView::releaseActiveConnections()
{
	for(const auto& c : sig_conns_){
		disconnect(c);
	}
	sig_conns_.clear();
}

void ProcessView::addSignalConnection(QMetaObject::Connection c)
{
	sig_conns_.push_back(c);
}

NAMESPACE_END