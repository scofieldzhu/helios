/******************************************************** 
* author: scofieldzhu
* time:2025/7/18
*******************************************************/
#include "line_sprite.h"
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include "sphere_sprite.h"

HELIOS_NAMESPACE_BEGIN

LineSprite::LineSprite(double point_size, double line_width)
    :line_source_(vtkSmartPointer<vtkLineSource>::New()),
    start_point_(std::make_unique<SphereSprite>()),
    end_point_(std::make_unique<SphereSprite>()),
    line_prop_(vtkSmartPointer<vtkProperty>::New())
{
    line_prop_->SetLineWidth(line_width);
    start_point_->setColor(Color::Red);
    start_point_->setRadius(point_size);
    start_point_->setCenter({0.0, 0.0, 0.0});
    end_point_->setColor(Color::Green);
    end_point_->setRadius(point_size);
    end_point_->setCenter({0.0, 0.0, 0.0});
    addChild(*start_point_);
    addChild(*end_point_);
    line_source_->SetPoint1(0.0, 0.0, 0.0);
    line_source_->SetPoint2(0.0, 0.0, 0.0);
}

LineSprite::~LineSprite()
{
    
}

void LineSprite::setStartPoint(const Point3& p)
{
    start_point_->setCenter(p);
    line_source_->SetPoint1(p);
    modified();
}

void LineSprite::setEndPoint(const Point3& p)
{
    end_point_->setCenter(p);
    line_source_->SetPoint2(p);
    modified();
}

void LineSprite::makeActors(Scene &scene)
{
    vtkNew<vtkPolyDataMapper> m;
    m->SetInputConnection(line_source_->GetOutputPort());
    vtkNew<vtkActor> a;
    a->SetMapper(m);
    a->SetProperty(line_prop_);
    addSceneProp(scene, a, "line");
}

NAMESPACE_END