/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/24
*******************************************************/
#ifndef __pathDesigner_h__
#define __simpleGreeter_h__

#include "IViewer.h"
#include "ISceneWidgetSource.h"
#include "IPathDesigner.h"

class PathDesignerDlg;

class PathDesigner : public QObject, public IPathDesigner, public ISceneWidgetSource, public IViewer
{
	Q_OBJECT
	Q_INTERFACES(IPathDesigner ISceneWidgetSource IViewer)
	Q_PLUGIN_METADATA(IID IPATH_DESIGNER_IID FILE "pathDesigner.json")
public:	
	void loadPaths(helios::SurgicalPathGroup& gp) override;
	helios::SurgicalPathGroup* getPathGroup() override{ return path_group_; }
	bool createView(QWidget* parent) override;
    QWidget* getViewWidget() override;   
    void setRenderWidget(helios::RenderWidget* sw) override;
    helios::RenderWidget* getRenderWidget() override;    
    void switchLanguage(const QString &lang_code) override;
	void onViewClosed() override;
	void setEventSink(IViewerEventSink* sink) override;
    PathDesigner(QObject* p = nullptr);
	~PathDesigner();

private:
	helios::SurgicalPathGroup* path_group_ = nullptr;
	PathDesignerDlg* dlg_ = nullptr;
    helios::RenderWidget* sw_ = nullptr;
};

#endif