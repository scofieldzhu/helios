/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/1/29
*******************************************************/
/***************************************************************
RenderPane - the base type for a virtual scene

The RenderPane can contain widgets (e.g. buttons), one or more
3D cursors, and can be connected to the Bubbles which represent
virtual 3D objects.

There is exactly one vtkRenderer associated with each RenderPane.
There can be more than one RenderPane inside a PaneFrame, just
as a vtkRenderWindow can support multiple vtkRenderers.  Usually,
however, the is one PaneFrame per RenderPane.
****************************************************************/
#ifndef __scene_h__
#define __scene_h__

#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkActor2D.h>
#include <vtkImageActor.h>
#include <vtkRenderer.h>
#include <vtkCallbackCommand.h>
#include "mirfak/core/mirfak_core_export.h"
#include "mirfak/core/mirfak_core_typedef.h"
#include "mirfak/basic/mobject.h"

class vtkCamera;
class vtkRenderWindow;
class vtkTexture;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_CORE_API Scene : public MObject 
{
	SCENE_DECL(Scene, MObject)
public:
    std::optional<Point3> pickWorldPointFromVolume(int x, int y)const;
    std::optional<Point3> grabWorldPointFromPixel(int x, int y)const;
	Point3 displayToWorld(const Point3& display_point)const;
	Point3 worldToDisplay(const Point3& world_point)const;
	Point3 viewToWorld(const Point3& view_point)const;
	Point3 worldToView(const Point3& world_point)const;
	Point3 viewToDisplay(const Point3& view_point)const;
	Point3 displayToView(const Point3& display_point)const;
	vtkRenderer* getRenderer() const;
	vtkCamera* getActiveCamera()const;
    void render();
	void modified();
	bool hasChangedSince(unsigned long since_time);
	unsigned long renderTime();
    void setViewport(double min_x, double min_y, double max_x, double max_y); 
	Arry4d getViewport()const;	
	void setBkgClr(const Color& color);
	void setBkgTexture(vtkTexture* t);
	Point2i getDisplayOrigin() const;
	Arry2i getDisplaySize()const;
	Arry4i getDisplayGeometry()const;
	bool hasSprite(const Sprite& s) const;
	const auto& spriteList()const { return sprite_list_; }
	Sprite* getSprite(const std::string& name)const;
	void resetView();	
	auto sceneWidget()const{ return scene_widget_; }
	vtkRenderWindow* getRenderWindow()const;
	void tearDown();	
	const std::string_view& name()const { return name_; }
	Scene(const std::string_view& name);		
	~Scene();

protected:
    friend class Sprite;
	friend class RenderWidget;	
	static void OnUpdateBkg(vtkObject* caller, unsigned long, void* clientData, void*);
	virtual void addToRenderWidget(RenderWidget& sw);
	virtual void removeFromRenderWidget();	
	void updateBkgForViewport();
	void addSprite(Sprite& s);
	void removeSprite(Sprite& s);
	void setSceneWidget(RenderWidget* s);
	void createBkg();
	void setTextureBkgEnabled(bool enabled);
	void setClrBkgEnabled(bool enabled);
	std::string_view name_;
	vtkSmartPointer<vtkRenderer> renderer_;
	vtkSmartPointer<vtkRenderer> bkg_ren_;
	vtkSmartPointer<vtkObject> modified_time_;
	vtkSmartPointer<vtkObject> render_time_;
	vtkSmartPointer<vtkActor2D> bkg_clr_actor_;
	vtkSmartPointer<vtkImageActor> bkg_image_actor_;
	int bkg_texture_dims_[3]{0,0,0};
	vtkSmartPointer<vtkCallbackCommand> bkg_cb_;
	SpriteList sprite_list_;
	RenderWidget* scene_widget_ = nullptr;
};

NAMESPACE_END

#endif