/*******************************************************
* author: scofieldzhu
* time:2025/10/30
*******************************************************/
#include "torus_sprite.h"
#include <vtkDiskSource.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

HELIOS_NAMESPACE_BEGIN

TorusSprite::TorusSprite(double inner_radius, double outer_radius, double thickness)
	:torus_source_(vtkSmartPointer<vtkDiskSource>::New()),
	extruder_(vtkSmartPointer<vtkLinearExtrusionFilter>::New())
{
	torus_source_->SetCenter(0.0, 0.0, 0.0);
	torus_source_->SetNormal(0.0, 0.0, 1.0);
	torus_source_->SetInnerRadius(inner_radius);
	torus_source_->SetOuterRadius(outer_radius);
 	torus_source_->SetRadialResolution(1);
 	torus_source_->SetCircumferentialResolution(120);

    extruder_->SetExtrusionTypeToNormalExtrusion();
    extruder_->SetVector(0.0, 0.0, thickness); 
    extruder_->CappingOn(); 
}

TorusSprite::~TorusSprite()
{
}

void TorusSprite::createSource(Scene& scene)
{
	extruder_->SetInputConnection(torus_source_->GetOutputPort());
}

void TorusSprite::makeActors(Scene& scene)
{
	vtkNew<vtkPolyDataMapper> m;
	m->SetInputConnection(extruder_->GetOutputPort());
	vtkNew<vtkActor> a;
	a->SetMapper(m);
	addSceneProp(scene, a, "torus");
}

void TorusSprite::setCenter(const Point3& pt)
{
	torus_source_->SetCenter(pt);
	modified();
}

Point3 TorusSprite::getCenter() const
{
	return torus_source_->GetCenter();
}

void TorusSprite::setNormal(const Vec3& n)
{
	const Vec3 cur_vec = extruder_->GetVector();
	double thickness = cur_vec.norm();
	Vec3 new_vec = n;
	new_vec.normalize();
	new_vec *= thickness;
	extruder_->SetVector(new_vec);
	torus_source_->SetNormal(n);
	modified();
}

Vec3 TorusSprite::getNormal() const
{
	return torus_source_->GetNormal();
}

NAMESPACE_END