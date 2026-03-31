/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/1/26
*******************************************************/
#ifndef __sprite_h__
#define __sprite_h__

/****************************************************************
Sprite - the base type for 3D virtual objects

An Sprite is a virtual object that can be displayed in one or more
RenderPanes simultaneously.

There can be many vtkActors associated with an Sprite. Each time
the Sprite is connected to a Scene, the Sprite will generate
a new set of actors for that pane.

Sprites can be built hierarchically, i.e. several Sprites can be
gathered together as Children of a single Parent and be manipulated
as a single unit.

To derive your own class from the Sprite base class, you must
override the 'MakeActors()' method, which generates a fresh tuple
of actors as well as any other entities that cannot be shared between
rendering contexts.  The 'makeActors()' method should call the
'newActorInfo()' method to create fresh actors.
****************************************************************/
#include <vtkSmartPointer.h>
#include <vtkActorCollection.h>
#include <vtkActor2DCollection.h>
#include "mirfak/basic/mobject.h"
#include "mirfak/basic/signal.hpp"
#include "mirfak/core/pick_info.h"
#include "mirfak/core/mirfak_core_export.h"
#include "mirfak/core/sprite_group.hpp"

class vtkCellPicker; 
class vtkRenderWindow;
class vtkPickingManager;
class vtkAssemblyPath;
class vtkAbstractPropPicker;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_CORE_API Sprite : public MObject
{
	SPRITE_DECL(Sprite, MObject)
public:
	Signal<Scene*, Sprite*, bool> connect_state_changed;
	Signal<Sprite*, bool> highlight_state_changed;
	Signal<Sprite*, bool> pick_state_changed;
	Signal<Sprite*, unsigned int> visible_state_changed;
    Signal<Sprite*> TransformChanged;
	static std::string GetPropName(vtkProp& prop);
	virtual PickInfoList getPickList(vtkCellPicker* picker, Scene& scene, int x, int y)const;
	void modified();
	void setName(const std::string& name);
	const auto& name()const { return name_; }
	auto master(){ return master_; }
	// void setPickable(bool flag);
	// bool pickable()const { return pickable_; }
	// void onPicked(PickInfo& info);
	// void onUnPicked(PickInfo* next_pick_info);
	//bool picked() const { return picked_; }
	virtual void highlight(const Color& highlight_clr);
	virtual void unhighlight();
	virtual void setVisibility(int v);
    virtual void setSceneVisibility(const Scene& s, int v);
    int getVisibility(const Scene& s)const;
	virtual void setOpacity(double opacity);
	virtual void setColor(const Color& color);
    virtual ColorOpt getColor()const;
	void setTransform(vtkTransform* t);
	vtkTransform* getTransform()const;
	virtual bool hasChangedSince(unsigned long since_mtime)const;
	auto renderTime() { return render_timestamp_.Get(); }
	void render();
	void connectScene(Scene& scene);
	void connectScenes(const SceneList& scene);
	void disconnectScene(Scene& scene);
	void disconnectScenes(const SceneList* panes);
	bool checkConnective(const Scene& pane) const;
	SceneList getDisplaySceneList()const;
	bool existsDisplayScene()const;
	bool addChild(Sprite& child);
	const Sprite* getChild(const std::string& name)const;
	Sprite* getChild(const std::string& name);
	bool hasChild(const Sprite& child)const;
	auto children()const{ return children_; }
	void removeChild(Sprite& child);
	void removeChildren();
	vtkActor* getFirstSceneActor(const Scene* scene)const;	
	vtkActor* getSceneActor(const std::string& name, const Scene* scene)const;	
	vtkSmartPointer<vtkActorCollection> getSceneActors(const Scene* scene, bool is_contain_child = false)const;	
	bool hasSceneProp(const Scene* scene, vtkProp* p)const;
	void getSceneProps(const Scene* scene, vtkPropCollection* result_props)const;

	virtual void startEditing(const Point2& pt);
	virtual void interacting(const Point2& pt);
	virtual void endEditing(const Point2& pt);
    bool isEditing()const{ return is_editing_; }
	virtual int computeEditState(Scene& s, int x, int y, int modify = 0);
	int getEditState()const;
    virtual const void* getEditStateData()const;

    vtkAssemblyPath* getAssemblyPath(double x, double y, double z, Scene* s, vtkAbstractPropPicker* picker);
    vtkPickingManager* getPickingManager(vtkRenderWindow* rw);
    virtual void registerPickers();
    virtual void unRegisterPickers();

    using SpriteGroupType = SpriteGroup<Sprite, SpriteSPtr>;
    SpriteGroupType* createComponentGroup(const std::string_view& name);    
    SpriteGroupType* getComponentGroup(const std::string_view& name);
    const SpriteGroupType* getComponentGroup(const std::string_view& name)const;
    void removeComponentGroup(const std::string_view& name);
    void clearComponentGroups();

	const std::string& objectUID()const{ return object_uid_; }

	Sprite();
	virtual ~Sprite();

protected:		
	void addSceneProp(Scene& scene, vtkProp* prop, const std::string& name);
	void removeSceneProp(Scene& scene, const std::string& name);
	void removeSceneProp(Scene& scene, vtkProp* a);
	virtual bool addToScene(Scene& scene);
	virtual void removeFromScene(Scene& scene);
	virtual void createSource(Scene& scene);
	virtual void makeActors(Scene& scene);
    void registerPicker(vtkRenderWindow* rw, vtkAbstractPropPicker* p);
	void unRegisterPicker(vtkRenderWindow* rw, vtkAbstractPropPicker* p);
    bool isRegisteredPicker(vtkRenderWindow* rw, vtkAbstractPropPicker* p);
	int edit_state_ = 0;
    bool is_editing_ = false;
	const std::string object_uid_;
    vtkSmartPointer<vtkObject> modifier_;    
    using SpriteGroupPtr = std::unique_ptr<SpriteGroupType>;
    std::map<const std::string_view, SpriteGroupPtr> component_groups_;

private:
	std::string name_;
	// bool pickable_ = true;
	// bool picked_ = false;
	std::optional<int> visibility_;  
    std::optional<Color> color_;
	std::optional<double> opacity_;
	SpriteList children_;
	vtkSmartPointer<vtkObject> render_timestamp_;
	vtkSmartPointer<vtkTransform> transform_;
	Sprite* master_ = nullptr;
	std::map<Scene*, vtkPropCollection*> scene_props_dict_;
	Scene* making_actor_scene_ = nullptr;
	bool source_created_ = false;	
    std::vector<vtkRenderWindow*> pick_rws_;
};

NAMESPACE_END

#endif