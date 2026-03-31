/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/25
*******************************************************/
#include "trackerToolManager.h"
#include "trackerToolManagerDlg.h"
using namespace mirfak;

TrackerToolManager::TrackerToolManager(QObject* p /*= nullptr*/)
	:QObject(p)
{}

TrackerToolManager::~TrackerToolManager()
{
	delete dlg_;
}

bool TrackerToolManager::createView(QWidget* parent)
{
	if(dlg_ == nullptr){
		dlg_ = new TrackerToolManagerDlg(parent);
	}
    if(sw_){
        dlg_->setRenderWidget(sw_);
    }	
	return true;
}

QWidget* TrackerToolManager::getViewWidget()
{
	return dlg_;
}

void TrackerToolManager::switchLanguage(const QString &lang_code)
{
}

void TrackerToolManager::setRenderWidget(RenderWidget* sw)
{
    sw_ = sw;
    if(dlg_){
        dlg_->setRenderWidget(sw);
    }    
}

RenderWidget* TrackerToolManager::getRenderWidget()
{
    return sw_;
}

void TrackerToolManager::onViewClosed()
{

}

void TrackerToolManager::setEventSink(IViewerEventSink* sink) 
{

}
