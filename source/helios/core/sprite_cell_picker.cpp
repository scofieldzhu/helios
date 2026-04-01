/******************************************************** 
* author: scofieldzhu
* time:2025/4/8
*******************************************************/
#include "sprite_cell_picker.h"
#include <vtkCellPicker.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include "sprite.h"
#include "scene.h"

HELIOS_NAMESPACE_BEGIN

SpriteCellPicker::SpriteCellPicker(const Sprite* target)
    :target_sprite_(target)
{}

PickInfoList SpriteCellPicker::pick(Scene& scene, int x, int y)
{
    vtkNew<vtkCellPicker> picker;    
    picker->SetTolerance(0.001);    
    const auto& all_sprites = scene.spriteList();
    SpriteList pickable_sprites;
    std::for_each(
        all_sprites.begin(),
        all_sprites.end(),
        [&pickable_sprites, &scene](Sprite* s){
            if(s->getVisibility(scene)){
                pickable_sprites.push_back(s);
            }
        }
    );
	if(pickable_sprites.empty() || picker->Pick(x, y, 0.0, scene.getRenderer()) == 0){
		return {};
	}
    auto is_excluded_sprite = [this](const Sprite* s)->bool{
        return std::find(excluded_sprites_.begin(), excluded_sprites_.end(), s) != excluded_sprites_.end();
    };
    Point3 camera_position = scene.getRenderer()->GetActiveCamera()->GetPosition();
    PickInfoList result_pick_info_list;        
    for(auto s : pickable_sprites){
        if(target_sprite_ != nullptr){
            if(target_sprite_ != s){
                continue;
            }
        }else if(is_excluded_sprite(s)){
            continue;
        }
        PickInfoList pick_info_list = s->getPickList(picker, scene, x, y);
        for(auto& pinfo : pick_info_list){
            pinfo.distance = pinfo.position.distanceTo(camera_position);
            pinfo.sprite = s;
            pinfo.scene = &scene;
            result_pick_info_list.push_back(pinfo);
        }         
        if(target_sprite_ && target_sprite_ == s){
            break;
        }
    }
    if(result_pick_info_list.size() > 1){
        std::sort(result_pick_info_list.begin(), result_pick_info_list.end(), [](const PickInfo& lhs, const PickInfo& rhs){ return lhs.distance < rhs.distance; });
        // if(respicklist[0].factory->pickPriority() > respicklist[1].factory->pickPriority()){ //swap the two            
        //     PickInfo tmp = respicklist[0];
        //     respicklist[0] = respicklist[1];
        //     respicklist[1] = tmp;
        // }
    }
    return result_pick_info_list;
}

NAMESPACE_END
