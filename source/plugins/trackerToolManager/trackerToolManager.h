/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/25
*******************************************************/
#ifndef __trackerToolManager_h__
#define __trackerToolManager_h__

#include "IViewer.h"
#include "ISceneWidgetSource.h"
#include "ITrackerToolManager.h"

class TrackerToolManagerDlg;

class TrackerToolManager : public QObject, public ITrackerToolManager, public ISceneWidgetSource, public IViewer
{
	Q_OBJECT
	Q_INTERFACES(ITrackerToolManager ISceneWidgetSource IViewer)
	Q_PLUGIN_METADATA(IID ITRACKER_TOOL_MANAGER_IID FILE "trackerToolManager.json")
public:	
    void setRenderWidget(helios::RenderWidget* sw) override;
    helios::RenderWidget* getRenderWidget() override;
	bool createView(QWidget* parent) override;
    QWidget* getViewWidget() override;    
    void switchLanguage(const QString &lang_code) override;
	void onViewClosed() override;
	void setEventSink(IViewerEventSink* sink) override;
    TrackerToolManager(QObject* p = nullptr);
	~TrackerToolManager();

private:
	TrackerToolManagerDlg* dlg_ = nullptr;
    helios::RenderWidget* sw_ = nullptr;
};

#endif