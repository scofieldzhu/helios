/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/4/18
*******************************************************/
#include "sprite_prop_picker.h"
#include <vtkPropPicker.h>
#include <vtkPropCollection.h>
#include <vtkRenderer.h>
#include "sprite.h"
#include "scene.h"

MIRFAK_NAMESPACE_BEGIN

namespace {
    Sprite* FindTargetSprite(Scene& scene, vtkProp* prop)
    {
        for(auto s : scene.spriteList()){
            if(s->getVisibility(scene)){
                vtkNew<vtkPropCollection> prop;
                s->getSceneProps(&scene, prop);
                if(prop->IsItemPresent(prop) != 0){
                    return s;
                }
            }
        }
        return nullptr;
    }
}

PickInfoList SpritePropPicker::pick(Scene& scene, int x, int y)
{
    vtkNew<vtkPropCollection> result_prop_collection;
    for(const Sprite* s : scene.spriteList()){
        if(s->getVisibility(scene)){
            s->getSceneProps(&scene, result_prop_collection);
        }
    }
    if(result_prop_collection->GetNumberOfItems() <= 0){
        return {};
    } 
    vtkNew<vtkPropPicker> picker;
    if(!picker->PickProp(x, y, scene.getRenderer(), result_prop_collection)){
        return {};
    }
    PickInfo result_pick_info;
    result_pick_info.prop = picker->GetViewProp();
    result_pick_info.sprite = FindTargetSprite(scene, result_pick_info.prop);
    result_pick_info.position = picker->GetSelectionPoint();
    result_pick_info.scene = &scene;
    return {result_pick_info};
}

NAMESPACE_END