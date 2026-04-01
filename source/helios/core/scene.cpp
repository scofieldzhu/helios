/******************************************************** 
* author: scofieldzhu
* time:2025/1/29
*******************************************************/
#include "scene.h"
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkCoordinate.h>
#include <vtkPolyData.h>
#include <vtkProperty2D.h>
#include <vtkCallbackCommand.h>
#include <vtkVolumePicker.h>
#include <vtkTexture.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include "helios/basic/log_service.h"
#include "render_widget.h"
#include "sprite.h"

HELIOS_NAMESPACE_BEGIN

Scene::Scene(const std::string_view& name)
	:name_(name),
	renderer_(vtkSmartPointer<vtkRenderer>::New()),	
    bkg_ren_(vtkSmartPointer<vtkRenderer>::New()),
	modified_time_(vtkSmartPointer<vtkObject>::New()),
	render_time_(vtkSmartPointer<vtkObject>::New())
{
	renderer_->GetActiveCamera()->ParallelProjectionOn();
	bkg_ren_->InteractiveOff();
	bkg_ren_->GetActiveCamera()->ParallelProjectionOn();
    createBkg();
}

Scene::~Scene()
{
    tearDown();
    if(scene_widget_){
        SPDLOG_CRITICAL("##########Must call removeScene prior to destructor called!");        
    }    
    //if(scene_widget_){
    //    scene_widget_->removeScene(*this);
    //}
}

std::optional<Point3> Scene::pickWorldPointFromVolume(int x, int y) const
{
    vtkNew<vtkVolumePicker> picker;
    if(picker->Pick(x, y, 0, renderer_)){
        return picker->GetPickPosition();
    }
    return std::nullopt;
}

std::optional<Point3> Scene::grabWorldPointFromPixel(int x, int y) const
{
    if(scene_widget_ == nullptr){
        return std::nullopt;
    }
    auto rw = renderer_->GetRenderWindow();
    auto warning_flag = rw->GetGlobalWarningDisplay();
    rw->SetGlobalWarningDisplay(0);
    rw->Render();
    float z;
    rw->GetZbufferData(x, y, x, y, &z);
    rw->SetGlobalWarningDisplay(warning_flag);
    if(z > 0.999){ //it's lies on far plane or infinite far.
        return std::nullopt;
    }
    return displayToWorld({(double)x, (double)y, (double)z});
}

Point3 Scene::displayToWorld(const Point3 &display_point) const
{        
    renderer_->SetDisplayPoint(display_point);
    renderer_->DisplayToWorld();
    auto wp = renderer_->GetWorldPoint();
    return Point3(wp[0] / wp[3], wp[1] / wp[3], wp[2] / wp[3]);
}

Point3 Scene::worldToDisplay(const Point3& world_point)const
{
    Arry4d wp = {world_point[0], world_point[1], world_point[2], 1.0};
    renderer_->SetWorldPoint(wp.data());
    renderer_->WorldToDisplay();
    return renderer_->GetDisplayPoint();
}

Point3 Scene::viewToWorld(const Point3& view_point) const
{
    renderer_->SetViewPoint(view_point[0], view_point[1], view_point[2]);
    renderer_->ViewToWorld();
    return renderer_->GetWorldPoint();
}

Point3 Scene::worldToView(const Point3& world_point) const
{
    renderer_->SetWorldPoint(world_point[0], world_point[1], world_point[2], 1.0);
    renderer_->WorldToView();        
    return renderer_->GetViewPoint();
}

Point3 Scene::viewToDisplay(const Point3& view_point) const
{
    renderer_->SetViewPoint(view_point[0], view_point[1], view_point[2]);
    renderer_->ViewToDisplay();
    return renderer_->GetDisplayPoint();
}

Point3 Scene::displayToView(const Point3& display_point) const
{
    renderer_->SetDisplayPoint(display_point[0], display_point[1], display_point[2]);
    renderer_->DisplayToView();
    return renderer_->GetViewPoint();
}

vtkRenderer* Scene::getRenderer() const
{
    return renderer_.Get();
}

void Scene::setViewport(double min_x, double min_y, double max_x, double max_y)
{
    auto clamped_min_x = std::clamp(min_x, 0.0, 1.0);
    auto clamped_min_y = std::clamp(min_y, 0.0, 1.0);
    auto clamped_max_x = std::clamp(max_x, 0.0, 1.0);
    auto clamped_max_y = std::clamp(max_y, 0.0, 1.0);
	renderer_->SetViewport(clamped_min_x, clamped_min_y, clamped_max_x, clamped_max_y);
	bkg_ren_->SetViewport(clamped_min_x, clamped_min_y, clamped_max_x, clamped_max_y);
}

Arry4d Scene::getViewport() const
{
    Arry4d vp;
    renderer_->GetViewport(vp.data());
    return vp;
}

void Scene::setBkgClr(const Color& clr)
{
    if(bkg_ren_ == nullptr){
        return;
    }
    if(bkg_image_actor_){
        setTextureBkgEnabled(false);
    }
    bkg_clr_actor_->GetProperty()->SetColor(clr.toValuesR().data());
    setClrBkgEnabled(true);
    bkg_clr_actor_->Modified();
    modified();
}

Point2i Scene::getDisplayOrigin()const
{
	return renderer_->GetOrigin();
}

Arry2i Scene::getDisplaySize()const
{
	return ToArray<int, 2>(renderer_->GetSize());
}

Arry4i Scene::getDisplayGeometry() const
{
    auto origin = getDisplayOrigin();
    auto size = getDisplaySize();
    return std::to_array({origin.x, origin.y, size[0], size[1]});
}

void Scene::modified()
{
	modified_time_->Modified();
    renderer_->Modified();
}

bool Scene::hasChangedSince(unsigned long since_mtime)
{
    if(modified_time_->GetMTime() > since_mtime){
		return true;
	}
    if(renderer_->GetMTime() > since_mtime){
		return true;
	}
	for(auto sprite : sprite_list_){
        if(sprite->hasChangedSince(since_mtime)){
			return true;
		}
    }
    return false;
}

unsigned long Scene::renderTime()
{
	return render_time_->GetMTime();
}

void Scene::render()
{
    if(scene_widget_){
        scene_widget_->render();
    }
}

Sprite* Scene::getSprite(const std::string& name) const
{
	auto it = std::find_if(sprite_list_.begin(), sprite_list_.end(), [&name](Sprite* s){
		return s->name() == name;
	});
    return it != sprite_list_.end() ? *it : nullptr;
}

void Scene::resetView()
{
	//if(CursorOnFlag)
	{
		//for cursor in self._Cursors:
		//  cursor.SetVisibility(self._Renderer,0)
	}
	renderer_->ResetCamera();
	//if(CursorOnFlag)
	{
		//for cursor in self._Cursors:
		//  cursor.SetVisibility(self._Renderer,1)
	}
	modified();
}

vtkCamera* Scene::getActiveCamera()const
{
	return renderer_->GetActiveCamera();
}

void Scene::addSprite(Sprite &s)
{
    if(!hasSprite(s)){
        sprite_list_.push_back(&s);
        modified();
    }
}

void Scene::removeSprite(Sprite& s)
{
	auto it = std::find(sprite_list_.begin(), sprite_list_.end(), &s);
    if(it != sprite_list_.end()){
		sprite_list_.erase(it);
        modified();
    }
}

bool Scene::hasSprite(const Sprite& s) const
{
    return std::find(sprite_list_.begin(), sprite_list_.end(), &s) != sprite_list_.end();
}

void Scene::setSceneWidget(RenderWidget* s)
{
	scene_widget_ = s;
}

void Scene::createBkg()
{
    vtkNew<vtkPoints> pts;
    pts->InsertNextPoint(0, 0, 0);   
    pts->InsertNextPoint(1, 0, 0);   
    pts->InsertNextPoint(1, 1, 0);   
    pts->InsertNextPoint(0, 1, 0);   
    vtkNew<vtkCellArray> cell_array;
    vtkIdType pt_indexes[4] = {0, 1, 2 ,3};
    cell_array->InsertNextCell(4, pt_indexes);
    vtkNew<vtkPolyData> polys;
    polys->SetPoints(pts);
    polys->SetPolys(cell_array);
    vtkNew<vtkPolyDataMapper2D> mapper;
    mapper->SetInputData(polys);
    vtkNew<vtkCoordinate> coord;
    coord->SetCoordinateSystemToNormalizedViewport();
    mapper->SetTransformCoordinate(coord);
    bkg_clr_actor_ = vtkSmartPointer<vtkActor2D>::New();
    bkg_clr_actor_->SetMapper(mapper);    
    bkg_clr_actor_->GetProperty()->SetColor(0.0, 0.0, 0.0);
    bkg_clr_actor_->GetProperty()->SetDisplayLocationToBackground();
    bkg_clr_actor_->PickableOff();
    bkg_ren_->AddActor2D(bkg_clr_actor_);
}

void Scene::tearDown()
{
    while(!sprite_list_.empty()){
        auto s = sprite_list_.back();
        s->disconnectScene(*this);
    };	
	sprite_list_.clear();
}

void Scene::addToRenderWidget(RenderWidget& sw)
{
    auto rw = sw.renderWindow();
    if(bkg_ren_){
        bkg_ren_->SetLayer(sw.GetBkgLayerIndexOfScene());
        rw->AddRenderer(bkg_ren_);
    }
    renderer_->SetLayer(sw.GetMainLayerIndexOfScene());
    rw->AddRenderer(renderer_);
    scene_widget_ = &sw;         
}

void Scene::removeFromRenderWidget()
{
    if(scene_widget_ == nullptr){ //not belong to any RenderWidget object!
        return;
    }
    auto rw = scene_widget_->renderWindow();
    rw->RemoveRenderer(renderer_);
    if(bkg_ren_){
        if(bkg_image_actor_){
            setTextureBkgEnabled(false);  // remove observer command!
        }
        rw->RemoveRenderer(bkg_ren_);
    }
    scene_widget_ = nullptr;
}

vtkRenderWindow* Scene::getRenderWindow() const
{
    return scene_widget_ ? scene_widget_->renderWindow() : nullptr;
}

void Scene::OnUpdateBkg(vtkObject* caller, unsigned long, void* clientData, void*)
{
    auto* self = static_cast<Scene*>(clientData);
    self->updateBkgForViewport();
}

void Scene::setBkgTexture(vtkTexture* t)
{    
    auto image_data = t->GetInput();
    if(image_data == nullptr){
        return;
    }
    setClrBkgEnabled(false);
    if(!bkg_image_actor_){
        bkg_image_actor_ = vtkSmartPointer<vtkImageActor>::New();
        bkg_ren_->AddActor(bkg_image_actor_);
    }
    bkg_image_actor_->SetInputData(image_data);
    image_data->GetDimensions(bkg_texture_dims_);
    double w = bkg_texture_dims_[0];
    double h = bkg_texture_dims_[1];
    auto* cam = bkg_ren_->GetActiveCamera();
    cam->ParallelProjectionOn();
    cam->SetParallelScale(h / 2.0);
    cam->SetPosition(w / 2.0, h / 2.0, 1.0);
    cam->SetFocalPoint(w / 2.0, h / 2.0, 0.0);
    cam->SetViewUp(0, 1, 0);
    if(!bkg_cb_){
        bkg_cb_ = vtkSmartPointer<vtkCallbackCommand>::New();
        bkg_cb_->SetClientData(this);
        bkg_cb_->SetCallback(&OnUpdateBkg);
        bkg_ren_->AddObserver(vtkCommand::StartEvent, bkg_cb_);
    }    
    modified();
}

void Scene::updateBkgForViewport()
{
    if(!bkg_image_actor_ || bkg_texture_dims_[0] <= 0 || bkg_texture_dims_[1] <= 0){
        return;
    }
    double vp[4];
    bkg_ren_->GetViewport(vp);
    int* win = bkg_ren_->GetRenderWindow()->GetSize();
    double Vw = std::max(1.0, (vp[2] - vp[0]) * win[0]);
    double Vh = std::max(1.0, (vp[3] - vp[1]) * win[1]);
    double A = Vw / Vh;
    double W = static_cast<double>(bkg_texture_dims_[0]);
    double H = static_cast<double>(bkg_texture_dims_[1]);
    double sContain = std::max(H * 0.5, W / (2.0 * A));
    double sCover   = std::min(H * 0.5, W / (2.0 * A));
    double s = true ? sCover : sContain;
    auto cam = bkg_ren_->GetActiveCamera();
    cam->ParallelProjectionOn();
    cam->SetParallelScale(s);
    cam->SetFocalPoint(W * 0.5, H * 0.5, 0.0);
    cam->SetPosition(W * 0.5, H * 0.5, 1.0);
    cam->SetViewUp(0.0, 1.0, 0.0);
}

void Scene::setTextureBkgEnabled(bool enabled)
{
    if(enabled){
        if(bkg_image_actor_ && !bkg_ren_->HasViewProp(bkg_image_actor_)){
            bkg_ren_->AddActor(bkg_image_actor_);
            bkg_ren_->AddObserver(vtkCommand::StartEvent, bkg_cb_);
        }
    }else{
        if(bkg_image_actor_ && bkg_ren_->HasViewProp(bkg_image_actor_)){
            bkg_ren_->RemoveActor(bkg_image_actor_);
            bkg_ren_->RemoveObserver(bkg_cb_);
        }
    }
}

void Scene::setClrBkgEnabled(bool enabled)
{
    if(enabled){
        if(bkg_clr_actor_ && !bkg_ren_->HasViewProp(bkg_clr_actor_)){
            bkg_ren_->AddActor2D(bkg_image_actor_);
        }
    }else{
        if(bkg_clr_actor_ && bkg_ren_->HasViewProp(bkg_clr_actor_)){
            bkg_ren_->RemoveActor2D(bkg_clr_actor_);
        }
    }
}

NAMESPACE_END