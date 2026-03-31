/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/24
*******************************************************/
#ifndef __IViewer_h__
#define __IViewer_h__

#include <QWidget>

class IViewerEventSink;

class IViewer
{
public:    
    virtual QWidget* getViewWidget() = 0;    
    virtual bool createView(QWidget* parent) = 0;
    virtual void onViewClosed() = 0;
    virtual void switchLanguage(const QString& lang_code) = 0;
    virtual void setEventSink(IViewerEventSink* sink) = 0;
    virtual ~IViewer() = default;    
};

#define IVIEWER_IID "mirfak.plugin.IViewer/1.0"

Q_DECLARE_INTERFACE(IViewer, IVIEWER_IID)

#endif