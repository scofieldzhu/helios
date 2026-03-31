/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/5/15
*******************************************************/
#ifndef __point_cloud_sprite_h__
#define __point_cloud_sprite_h__

#include "mirfak/sprites/mirfak_sprites_export.h"
#include "mirfak/core/sprite.h"

class vtkPoints;
class vtkPolyData;
class vtkSphereSource;
class vtkGlyph3D;
class vtkVertexGlyphFilter;

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_SPRITES_API PointCloudSprite : public Sprite
{
	SPRITE_DECL(PointCloudSprite, Sprite)
public:  
    enum PointType{
        PT_VERTEX,
        PT_SPHERE
    };
    void setPointSize(float size);
    float pointSize()const{ return point_size_; }
    void setPoints(vtkPoints* pts);
    void setPoints(const Pt3List& pts);
    Pt3List getPoints()const;
    vtkActor* getCloudActor(Scene& s)const;
    auto pointType()const{ return point_type_; }
    PointCloudSprite(PointType type = PT_VERTEX);
    ~PointCloudSprite();

private:
    void createSource(Scene& s) override;
    void makeActors(Scene& s) override;
    vtkSmartPointer<vtkPolyData> polydata_;
    vtkSmartPointer<vtkSphereSource> sphere_point_;
    vtkSmartPointer<vtkGlyph3D> glyph_source_;
    vtkSmartPointer<vtkVertexGlyphFilter> vgly_filter_;
    PointType point_type_;
    float point_size_;
};

NAMESPACE_END

#endif