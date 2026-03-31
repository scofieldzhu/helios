/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/9/12
*******************************************************/
#ifndef __handpiece_sprite_h__
#define __handpiece_sprite_h__

#include "mirfak/navigation/drill_sprite.h"
#include "mirfak/navigation/tracker_tool_config.h"

class vtkTransformPolyDataFilter;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_NAVIGATION_API HandpieceSprite : public Sprite
{
    SPRITE_DECL(HandpieceSprite, Sprite)
public:    
    void switchDrill(const QString& drill_name);
    DrillSprite* getDrill();
    bool bindTrackerTool(const QString& tool_name);
    std::optional<QString> getBoundTrackerTool()const;
    void updateTransform();
    HandpieceSprite();
    ~HandpieceSprite();

private:
    vtkSmartPointer<vtkPolyData> readModelData(const QString& model_filepath)const;
    Point3 calcDrillTipOffset(const DrillConfig& drill_model)const;
    void makePolyDataEmpty(vtkPolyData* data);
    void createSource(Scene& scene) override;
	void makeActors(Scene& scene) override;
    void moveToBitDrill(double drill_length);
    void updateSourceModelData();
    void updateToolCalibrationMatrix(const Point3& tip_offset);
    vtkSmartPointer<vtkPolyData> source_model_polydata_;
    vtkSmartPointer<vtkTransformPolyDataFilter> transform_data_filter_;
    std::unique_ptr<DrillSprite> drill_;    
    std::optional<TrackerToolConfig> bound_tool_config_opt_;
    std::optional<DrillConfig> calib_drill_config_opt_;
    Point3 current_mark_point1_, current_mark_point2_, current_mark_point3_;
};

NAMESPACE_END

#endif