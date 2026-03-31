/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/5/21
*******************************************************/
#ifndef __render_widget_h__
#define __render_widget_h__

#include <QVTKOpenGLNativeWidget.h>
#include "mirfak/core/mirfak_core_export.h"
#include "mirfak/core/mirfak_core_typedef.h"

class vtkTexture;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_CORE_API RenderWidget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT
public:
    static Scene* RendererToScene(vtkRenderer* ren);
    static int GetBkgLayerIndexOfScene();
    static int GetMainLayerIndexOfScene();
    static int GetHudLayerIndexOfScene();
    void render();
    void setBkgColor(const Color& clr);
    void setBkgTexture(vtkTexture* t);
    Color getColor()const;
    void addScene(Scene& s);
    void removeScene(Scene& s);
    void removeAll();
    const SceneRepository& sceneRepository()const;
    RenderWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~RenderWidget();

private:    
    void initBackground();    
    vtkSmartPointer<vtkRenderer> bkg_ren_;
    std::unique_ptr<SceneRepository> scene_respository_;
    static std::vector<RenderWidget*> stAlivedWidgets;
};

NAMESPACE_END

#endif