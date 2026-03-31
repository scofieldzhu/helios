/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/9/12
*******************************************************/
#ifndef __polydata_cutter_sprite_h__
#define __polydata_cutter_sprite_h__

#include "mirfak/core/sprite.h"
#include "mirfak/sprites/mirfak_sprites_export.h"

class vtkPlane;
class vtkAlgorithmOutput;
class vtkCutter;
class vtkCleanPolyData;
class vtkTransformPolyDataFilter;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API PolyDataCutterSprite : public Sprite
{
    SPRITE_DECL(PolyDataCutterSprite, Sprite)
public:
    void setInputPolyData(vtkAlgorithmOutput* polys);
    void setPolyDataTransform(vtkTransform* trans);
    void setCutPlaneEquation(vtkPlane* plane);
    PolyDataCutterSprite();
    ~PolyDataCutterSprite();

private:
    void createSource(Scene& scene) override;    
    void makeActors(Scene& scene) override;    
    vtkSmartPointer<vtkCutter> cutter_;
    vtkSmartPointer<vtkCleanPolyData> clean_filter_;
    vtkSmartPointer<vtkTransformPolyDataFilter> trans_filter_;
};

NAMESPACE_END

#endif