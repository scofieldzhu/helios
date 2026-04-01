/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/4/21
*******************************************************/
#ifndef __plane_cutter_sprite_h__
#define __plane_cutter_sprite_h__

#include "helios/sprites/helios_sprites_export.h"
#include "helios/core/sprite.h"

class vtkPlane;
class vtkCutter;
class vtkPlaneSource;
class vtkCleanPolyData;

HELIOS_NAMESPACE_BEGIN

class HELIOS_SPRITES_API PlaneCutterSprite : public Sprite
{
    SPRITE_DECL(PlaneCutterSprite, Sprite)
public:
    
    Point3 getCenter()const;
    // void updatePosition(const Point3& pos);
    // double getCurrentDistance(CuboidPlanes::PlaneType planetype)const;
    // int32_t getCurrentStepNum(double steplen, CuboidPlanes::PlaneType planetype)const;
    void setLineWidth(double w);
    auto lineWidth()const{ return line_width_; }
    bool hasChangedSince(unsigned long since_mtime) const override;
    void setTargetPlaneSource(vtkSmartPointer<vtkPlaneSource> plane_source);
    void setCutPlane(vtkSmartPointer<vtkPlane> cut_plane);  
    using PointCouple = std::array<Point3, 2>;
    std::optional<PointCouple> getLinePoints()const;
    PlaneCutterSprite();
    ~PlaneCutterSprite();

private:    
    void createSource(Scene& scene) override;    
    void makeActors(Scene& scene) override;    
    vtkSmartPointer<vtkPlane> cut_plane_;
    vtkSmartPointer<vtkPlaneSource> plane_source_;
    vtkSmartPointer<vtkCutter> cutter_;
    vtkSmartPointer<vtkCleanPolyData> clean_filter_;
    double line_width_ = 1.0;
};

//SIMPLE_EVENT_CLS(PlaneCutterPosChangedEvent)

NAMESPACE_END

#endif
