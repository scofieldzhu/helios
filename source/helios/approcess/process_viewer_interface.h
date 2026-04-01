/******************************************************** 
* author: scofieldzhu
* time:2026/2/25
*******************************************************/
#ifndef __process_view_interface_h__
#define __process_view_interface_h__

#include <QWidget>
#include "helios/helios_nsp.h"

HELIOS_NAMESPACE_BEGIN

class ProcessView;

class IProcessViewer
{
public:    
    virtual ProcessView* getViewWidget() = 0;    
    virtual bool createView(QWidget* parent) = 0;
    virtual void onViewClosed() = 0;
    virtual void switchLanguage(const QString& lang_code) = 0;
    virtual void setViewModel(IViewerEventSink* sink) = 0;
    virtual ~IProcessViewer() = default;    
};

NAMESPACE_END

#define IVIEWER_IID "helios.plugin.IViewer/1.0"

Q_DECLARE_INTERFACE(IViewer, IVIEWER_IID)

#endif