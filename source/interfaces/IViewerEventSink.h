/*******************************************************
* author: scofieldzhu
* time:2026/1/13
*******************************************************/
#ifndef __IViewerEventSink_h__
#define __IViewerEventSink_h__

#include <QString>
#include <QVariantMap>

class IViewer;

class IViewerEventSink
{
public:    
    // 插件状态变化
    virtual void onViewerStateChanged(IViewer* sender, const QString& state) = 0;
    
    // 插件请求显示消息
    virtual void onViewerMessage(IViewer* sender, int level, const QString& message) = 0;
    
    // 插件数据就绪（如 DICOM 加载完成）
    virtual void onDataReady(IViewer* sender, const QVariantMap& data) = 0;
    
    // 插件请求执行某个动作
    virtual void onActionRequested(IViewer* sender, const QString& action, const QVariantMap& params) = 0;

    virtual ~IViewerEventSink() = default;
};

namespace viewer_msg_level{
    constexpr int kInfo = 0;
    constexpr int kWarn = 1;
    constexpr int kError= 2;
}

#endif