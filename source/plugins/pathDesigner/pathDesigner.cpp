/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/24
*******************************************************/
#include "pathDesigner.h"
#include "pathDesignerDlg.h"

using namespace mirfak;

PathDesigner::PathDesigner(QObject* p /*= nullptr*/)
	:QObject(p)
{}

PathDesigner::~PathDesigner()
{
	delete dlg_;
}

void PathDesigner::loadPaths(mirfak::SurgicalPathGroup& gp)
{
	path_group_ = &gp;
}

bool PathDesigner::createView(QWidget* parent)
{
	if(path_group_ == nullptr){
		return false;
	}
	if(dlg_ == nullptr){
		dlg_ = new PathDesignerDlg(parent, this);
	}
    if(sw_){
        dlg_->setRenderWidget(sw_);
    }	
	return true;
}

QWidget* PathDesigner::getViewWidget()
{
	return dlg_;
}

void PathDesigner::setRenderWidget(RenderWidget *sw)
{
    sw_ = sw;
    if(dlg_){
        dlg_->setRenderWidget(sw_);
    }
}

RenderWidget* PathDesigner::getRenderWidget()
{
    return sw_;
}

void PathDesigner::switchLanguage(const QString &lang_code)
{
}

void PathDesigner::onViewClosed()
{

}

void PathDesigner::setEventSink(IViewerEventSink* sink) 
{

}