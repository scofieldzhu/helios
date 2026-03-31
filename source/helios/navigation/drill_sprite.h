/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/9/12
*******************************************************/
#ifndef __drill_sprite_h__
#define __drill_sprite_h__

#include "mirfak/core/sprite.h"
#include "mirfak/navigation/drill_config.h"
#include "mirfak/navigation/mirfak_navigation_export.h"

class vtkPolyData;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_NAVIGATION_API DrillSprite : public Sprite
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