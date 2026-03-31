/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/12/2
*******************************************************/
#ifndef ___ISceneWidgetSource_h_
#define ___ISceneWidgetSource_h_

#include "mirfak/core/render_widget.h"

class ISceneWidgetSource
{
public:    
    virtual void setRenderWidget(mirfak::RenderWidget* sw) = 0;
    virtual mirfak::RenderWidget* getRenderWidget() = 0;
    virtual ~ISceneWidgetSource() = default;    
};

#define ISCENE_WIDGET_SOURCE_IID "mirfak.plugin.ISceneWidgetSource/1.0"

Q_DECLARE_INTERFACE(ISceneWidgetSource, ISCENE_WIDGET_SOURCE_IID)

#endif