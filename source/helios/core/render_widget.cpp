/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/5/21
*******************************************************/
#include "render_widget.h"
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
//#include <vtkInformation.h>
//#include "vtkInformationObjectBaseKey.h"
#include "scene.h"
#include "scene_repository.h"
#include "camera_interactor_style.h"

HELIOS_NAMESPACE_BEGIN

namespace{
    constexpr int kBkgRenLayer = 0;
    constexpr int kSceneBkgRenLayer = 1;
    constexpr int kSceneMainRenLayer = 2;
    constexpr int kHudRenLayer = 3;
}

int RenderWidget::GetBkgLayerIndexOfScene()
{
    return kSceneBkgRenLayer;
}

int RenderWidget::GetMainLayerIndexOfScene()
{
    return kSceneMainRenLayer;
}

int RenderWidget::GetHudLayerIndexOfScene()
{
    return kHudRenLayer;
}

std::vector<RenderWidget*> RenderWidget::stAlivedWidgets;

RenderWidget::RenderWidget(QWidget* parent, Qt::WindowFlags f)
    :QVTKOpenGLNativeWidget(parent, f),
    bkg_ren_(vtkSmartPointer<vtkRenderer>::New()),
    scene_respository_(std::make_unique<SceneRepository>())
{
    stAlivedWidgets.push_back(this);
    renderWindow()->SetNumberOfLayers(4);
    initBackground();    
    vtkNew<CameraInteractorStyle> default_interactor_style;
    interactor()->SetInteractorStyle(default_interactor_style);
}

RenderWidget::~RenderWidget()
{
    stAlivedWidgets.erase(std::find(stAlivedWidgets.begin(), stAlivedWidgets.end(), this));
}

void RenderWidget::initBackground()
{
    bkg_ren_->SetBackground(1.0, 0.0, 0.0);
    bkg_ren_->SetViewport(0, 0, 1, 1);
    bkg_ren_->SetLayer(kBkgRenLayer);
    bkg_ren_->EraseOn();
    bkg_ren_->InteractiveOff();
    renderWindow()->AddRenderer(bkg_ren_);
}

void RenderWidget::setBkgColor(const Color& clr)
{
    bkg_ren_->SetBackground(clr.toValuesR().data());
}

Color RenderWidget::getColor()const
{
    auto clr = bkg_ren_->GetBackground();
    return {clr[0], clr[1], clr[2]};
}

void RenderWidget::render()
{
    renderWindow()->Render();
}

void RenderWidget::addScene(Scene& s)
{
    if(!scene_respository_->existScene(s)){
        s.addToRenderWidget(*this);    
        scene_respository_->addScene(s);
    }    
}

void RenderWidget::removeScene(Scene& s)
{
    if(s.scene_widget_ == this){
        s.removeFromRenderWidget();
        scene_respository_->removeScene(s);
    }         
}

Scene* RenderWidget::RendererToScene(vtkRenderer* ren)
{
    for(auto sw : RenderWidget::stAlivedWidgets){
        if(auto s = sw->scene_respository_->getScene(ren)){
            return s;
        }        
    }
    return nullptr;
}

const SceneRepository& RenderWidget::sceneRepository () const
{
    return *scene_respository_;
}

void RenderWidget::removeAll()
{
    while(!scene_respository_->empty()){
        auto s = scene_respository_->back();
        removeScene(*s);
    }
}

void RenderWidget::setBkgTexture(vtkTexture* t)
{
    bkg_ren_->SetTexturedBackground(true);
    bkg_ren_->SetBackgroundTexture(t);
}

NAMESPACE_END