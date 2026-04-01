/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/9/9
*******************************************************/
#ifndef __surgical_path_sprite_h__
#define __surgical_path_sprite_h__

#include "helios/core/sprite.h"
#include "helios/sprites/helios_sprites_export.h"
#include "helios/core/sprite_group.hpp"

class vtkCylinderSource;
class vtkConeSource;
class vtkPolyData;
class vtkAppendPolyData;
class vtkCleanPolyData;
class vtkAlgorithmOutput;
class vtkCellPicker;

HELIOS_NAMESPACE_BEGIN

class SphereSprite;

class HELIOS_SPRITES_API SurgicalPathSprite : public Sprite
{
    SPRITE_DECL(SurgicalPathSprite, Sprite)
public:
    double getMinimumPathLength()const;
    void setCustomTailPolys(vtkPolyData* tail_polys, double length);
    void setStartPoint(const Point3& pt);
    const auto& startPoint()const{ return start_point_; }
    void setEndPoint(const Point3& pt);
    const auto& endPoint()const{ return end_point_; }
    void setPoints(const Point3& s_pt, const Point3& e_pt);
    double getLength()const;
    double getRadius()const;
    vtkAlgorithmOutput* getPolyData()const;
    void startEditing(const Point2& pt) override;
	void endEditing(const Point2& pt) override;
    enum EditState{
        ES_NONE,
        ES_OUTSIDE,
        ES_HOVER,
        ES_START_POINT_SELECTED,
        ES_END_POINT_SELECTED,        
    };
	int computeEditState(Scene& s, int x, int y, int modify = 0) override;
    SurgicalPathSprite(double radius);
    ~SurgicalPathSprite();

private:    
    Pt3Arry6 getBoundboxWorldPoints()const;
    void setLength(double length);
    void createSource(Scene& scene) override;
	void makeActors(Scene& scene) override;
    vtkSmartPointer<vtkTransform> calcTransform(const Point3& bottom_center, const Point3& tip_pt)const;
    vtkSmartPointer<vtkCylinderSource> cylinder_source_;
    vtkSmartPointer<vtkConeSource> cone_source_;
    vtkSmartPointer<vtkPolyData> custom_tail_polys_;
    vtkSmartPointer<vtkAppendPolyData> polys_appender_;
    vtkSmartPointer<vtkCleanPolyData> clean_polys_;
    double tail_model_length_ = 0.0;
    Point3 start_point_;
    Point3 end_point_;
    std::unique_ptr<SphereSprite> start_ctrl_sphere_;
    std::unique_ptr<SphereSprite> end_ctrl_sphere_;
    Pt3Arry6 boundbox_points_;
    using PickerPtr = vtkSmartPointer<vtkCellPicker>;
    std::map<const Scene*, PickerPtr> picker_dict_;
};

using SurgicalPathGroup = SpriteGroup<SurgicalPathSprite>;

NAMESPACE_END

#endif