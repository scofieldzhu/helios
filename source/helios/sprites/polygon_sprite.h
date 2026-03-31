/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/6/19
*******************************************************/
#ifndef __polygon_sprite_h__
#define __polygon_sprite_h__

#include "mirfak/sprites/mirfak_sprites_export.h"
#include "mirfak/core/sprite.h"

class vtkPolyData;
class vtkLineSource;
class vtkPoints;
class vtkCellArray;
class vtkFloatArray;
class vtkProperty;
//class vtkPropPicker;
class vtkViewProp;
class vtkUnsignedCharArray;
class vtkIdTypeArray;
class vtkSelection;
class vtkHardwareSelector;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API PolygonSprite : public Sprite
{
    SPRITE_DECL(PolygonSprite, Sprite)
public:
    std::vector<int> getControlPointIdsFromSelection(vtkHardwareSelector* selector, vtkSelection* selection);
    void setLineWidth(double w);
    void setClosed(bool flag);
    bool closed()const{ return closed_; }
    void setPoints(const Pt3List& pts);
    Pt3Opt getPoint(int id)const;
    void setPoint(int id, Point3& pt);
    Pt3List getPoints()const;
    void appendPoint(const Point3& pt);
    void insertPoint(const Point3& pt, int pos);
    void registerPickers() override;
	void unRegisterPickers() override;
    //vtkAbstractPropPicker* getPicker();
    void setControlPointColor(const Color& clr);
    const auto& controlPointColor()const{ return ctrl_point_clr_; }
    void setControlPointSelectColor(const Color& clr);
    const auto& controlPointSelectColor()const{ return ctrl_point_select_clr_; }
    void setControlPointSelected(int index);
    int selectedControlPointId()const{ return selected_control_point_id_; }
    Pt3Opt getSelectedPoint()const;
    //int getPickedControlPoint(Scene& s, int x, int y)const;
    PolygonSprite();
    ~PolygonSprite();

private:
    bool addToScene(Scene& scene) override;
	void createSource(Scene& scene) override;
	void makeActors(Scene& scene) override;
    void makePolygonActor(Scene& scene);
    void makeEndLineActor(Scene& scene);
    void updatePolydata();
    void updateCloseLineData();
    bool closed_ = true;
    vtkSmartPointer<vtkPoints> points_;
    vtkSmartPointer<vtkCellArray> lines_;
    vtkSmartPointer<vtkPolyData> polys_;
    vtkSmartPointer<vtkPolyData> gly_polys_;
    vtkSmartPointer<vtkUnsignedCharArray> gly_colors_;
    vtkSmartPointer<vtkLineSource> close_line_source_;
    vtkSmartPointer<vtkFloatArray> cam_dir_array_;
    vtkSmartPointer<vtkProperty> line_property_;
    //vtkSmartPointer<vtkCellPicker> picker_;    
    vtkSmartPointer<vtkIdTypeArray> id_array_;
    int selected_control_point_id_ = -1;
    Color ctrl_point_clr_ = Color::Green;
    Color ctrl_point_select_clr_ = Color::Yellow;
};

NAMESPACE_END

#endif