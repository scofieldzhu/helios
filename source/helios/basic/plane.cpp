/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2023)
*******************************************************/
#include "plane.h"
#include <vtkPlane.h>
#include <vtkNew.h>

HELIOS_NAMESPACE_BEGIN

Plane::Plane(const Point3 &pt, const Vec3 &n)
    :normal(n),
    point(pt)
{
}

Pt3Opt Plane::CalcCenterOfTriplePlanes(const Plane& p1, const Plane& p2, const Plane& p3)
{
    auto line_opt = p1.intersectWithPlane(p2);
    if(!line_opt){
        return std::nullopt;
    }
    return p3.intersectWithLine(line_opt.value());
}

LineOpt Plane::intersectWithPlane(const Plane &rhs) const
{
    const auto& pt1 = point;
    const auto& n1  = normal;
    const auto& pt2 = rhs.point;
    const auto& n2  = rhs.normal;        
    const auto& vec = n1.cross(n2);
    const auto& v1  = n1.cross(vec);
    auto pt_opt = Plane(pt2, n2).intersectWithLine({pt1, v1});
    if(pt_opt){
        return Line{pt_opt.value(), vec};
    }
    return std::nullopt;
}

Point3 Plane::projectPoint(const Point3& pt) const
{
    vtkNew<vtkPlane> self;
    self->SetOrigin(point);
    self->SetNormal(normal);
    Point3 proj;
    self->ProjectPoint(pt, proj);
    return proj;
    // double t = normal[0] * point[0] - normal[0] * pt[0] + normal[1] * point[1] - normal[1] * pt[1] + normal[2] * point[2] - normal[2] * pt[2];
    // double div_factor = normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2];
    // if(fabs(div_factor) < kZeroTolerance)
    //     div_factor = kZeroTolerance;
    // t = t / div_factor;
    // return Point3(pt[0] + normal[0] * t, pt[1] + normal[1] * t, pt[2] + normal[2] * t);
}

double Plane::getProjectDistance(const Point3& pt) const
{
    return pt.distanceTo(projectPoint(pt));
}

Pt3Opt Plane::intersectWithLine(const Line& line)const
{
    vtkNew<vtkPlane> self;
    self->SetOrigin(point);
    self->SetNormal(normal);
    auto pt1 = line.origin;
    auto pt2 = line.origin + line.normal.normalized() * 1000.0;
    double t = 0.0;
    Point3 intersection;
    if(self->IntersectWithLine(pt1, pt2, t, intersection) == 0){
        pt2 = line.origin - line.normal.normalized() * 1000.0;
        if(self->IntersectWithLine(pt1, pt2, t, intersection) == 0){
            return std::nullopt;
        }
    }
    return intersection;
    // const auto& pt1 = line.origin_pt;
    // const auto& n1  = line.normal;
    // const auto& pt2 = point;
    // const auto& n2  = normal;    
    // Vec3 vec = pt2 - pt1;
    // auto t = n2.dot(vec);
    // auto t1 = n1.dot(n2); 
    // if(fabs(t1) < kZeroTolerance)
    //     t1 = kZeroTolerance;
    // t = t / t1;
    // return (pt1 + n1 * t);
}

NAMESPACE_END