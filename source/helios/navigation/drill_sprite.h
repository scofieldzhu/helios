/*******************************************************
* author: scofieldzhu
* time:2025/9/12
*******************************************************/
#ifndef __drill_sprite_h__
#define __drill_sprite_h__

#include "helios/core/sprite.h"
#include "helios/navigation/drill_config.h"
#include "helios/navigation/helios_navigation_export.h"

class vtkPolyData;

HELIOS_NAMESPACE_BEGIN

class HELIOS_NAVIGATION_API DrillSprite : public Sprite
{
    SPRITE_DECL(DrillSprite, Sprite)
public:
    void rotate();
    void loadConfig(const DrillConfig& cf);
    const auto& config()const{ return current_config_; }
    DrillSprite();
    ~DrillSprite();

private:
    void makePolyDataEmpty(vtkPolyData* data);
    void updateModelData();
    void createSource(Scene& scene) override;
	void makeActors(Scene& scene) override;
    DrillConfig current_config_;
    vtkSmartPointer<vtkPolyData> current_model_data_;
};

NAMESPACE_END

#endif