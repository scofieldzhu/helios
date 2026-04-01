/*******************************************************
* author: scofieldzhu
* time:2025/5/15
*******************************************************/
#ifndef __point_cloud_sprite_h__
#define __point_cloud_sprite_h__

#include "helios/sprites/helios_sprites_export.h"
#include "helios/core/sprite.h"

class vtkPoints;
class vtkPolyData;
class vtkSphereSource;
class vtkGlyph3D;
class vtkVertexGlyphFilter;

HELIOS_NAMESPACE_BEGIN

class HELIOS_SPRITES_API PointCloudSprite : public Sprite
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