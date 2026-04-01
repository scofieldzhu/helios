/******************************************************** 
* author: scofieldzhu
* time:2025/12/2
*******************************************************/
#ifndef ___ISceneWidgetSource_h_
#define ___ISceneWidgetSource_h_

#include "helios/core/render_widget.h"

class ISceneWidgetSource
{
public:    
    virtual void setRenderWidget(helios::RenderWidget* sw) = 0;
    virtual helios::RenderWidget* getRenderWidget() = 0;
    virtual ~ISceneWidgetSource() = default;    
};

#define ISCENE_WIDGET_SOURCE_IID "helios.plugin.ISceneWidgetSource/1.0"

Q_DECLARE_INTERFACE(ISceneWidgetSource, ISCENE_WIDGET_SOURCE_IID)

#endif