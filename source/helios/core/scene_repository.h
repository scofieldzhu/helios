/*******************************************************
* author: scofieldzhu
* time:2026/2/3
*******************************************************/
#ifndef __scene_repository_h__
#define __scene_repository_h__

#include "helios/core/helios_core_export.h"
#include "helios/core/helios_core_typedef.h"

class vtkRenderer;

HELIOS_NAMESPACE_BEGIN

class HELIOS_CORE_API SceneRepository
{
public:
	void addScene(Scene& s);
	void removeScene(Scene& s);
    using CheckFuncType = std::function<bool(Scene* s)>;
	Scene* getScene(CheckFuncType check_func)const;
	Scene* getScene(const std::string_view& name)const;
    Scene* getAxialScene()const;
    Scene* getCoronalScene()const;
    Scene* getSagittalScene()const;
    Scene* getOrthoScene()const;
	template<class T>
	T* findConcreteScene(const std::string_view& name_val = "")const;
    Scene* getScene(int id)const;
    Scene* back()const;
    void popBack();
    bool empty()const{ return managed_scenes_.empty(); }
    std::size_t getNumberOfScenes()const{ return managed_scenes_.size(); }
    Scene* getScene(vtkRenderer* ren)const;
    bool existScene(const Scene& s)const;
	SceneRepository();
	~SceneRepository() = default;

private:
	SceneList managed_scenes_;
};

template<class T>
T* SceneRepository::findConcreteScene(const std::string_view& name_val) const
{
    static_assert(std::is_base_of_v<Scene, T>);
    for(auto s : managed_scenes_){
        if(!s->isA(T::GetClsTypeName())){
            continue;
        }
        if(name_val.empty() || s->name() == name_val){
            return T::SafeDownCast(s);
        }
    }
    return nullptr;
}

NAMESPACE_END

#endif
