/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2024-2025)
* author: zhucg
* time:2025/10/31
*******************************************************/
#include "arrow_sprite.h"
#include <vtkArrowSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkActor.h>
#include <vtkTransform.h>

HELIOS_NAMESPACE_BEGIN

ArrowSprite::ArrowSprite(double shaft_radius, double tip_radius, double tip_len)
	:arrow_source_(vtkSmartPointer<vtkArrowSource>::New()),
	trans_poly_filter_(vtkSmartPointer<vtkTransformPolyDataFilter>::New())
{
	arrow_source_->SetShaftRadius(shaft_radius);
	arrow_source_->SetTipRadius(tip_radius);
	arrow_source_->SetTipLength(tip_len);
	arrow_source_->SetShaftResolution(100);
	arrow_source_->SetTipResolution(100);
}

ArrowSprite::~ArrowSprite()
{

}

void ArrowSprite::createSource(Scene& scene)
{
	trans_poly_filter_->SetInputConnection(arrow_source_->GetOutputPort());
	transformData();
}

void ArrowSprite::makeActors(Scene& scene)
{
	vtkNew<vtkPolyDataMapper> m;
	m->SetInputConnection(trans_poly_filter_->GetOutputPort());
	vtkNew<vtkActor> a;
	a->SetMapper(m);
	addSceneProp(scene, a, "arrow");
}

void ArrowSprite::setLength(double len)
{
	if(length_ != len && len > 0){
		length_ = len;
		transformData();
		modified();
	}
}

void ArrowSprite::setStartPoint(const Point3& pt)
{
	if(start_point_ != pt){
		start_point_ = pt;
		transformData();
		modified();
	}	
}

void ArrowSprite::setDirection(const Vec3& v)
{
	if(v != direction_){
		direction_ = v;
		transformData();
		modified();
	}
}

void ArrowSprite::transformData()
{
	Vec3 origin_vec(1.0, 0.0, 0.0);
	Vec3 cross_vec = origin_vec.crossed(direction_);
	auto s = cross_vec.norm();
	double d = origin_vec.dot(direction_);
	vtkNew<vtkTransform> t;
	t->Identity();
	t->PostMultiply();
	if(IsZero(s)){
		if(d < 0.0){
			t->RotateWXYZ(180.0, 0.0, 0.0, 1.0);
		}
	}else{
		cross_vec.normalize();
		double angle = RadianToDegree(std::atan2(s, d));
		t->RotateWXYZ(angle, cross_vec);
	}
	t->Scale(length_, length_, length_);
	t->Translate(start_point_);
	trans_poly_filter_->SetTransform(t);
	trans_poly_filter_->Modified();
}

NAMESPACE_END