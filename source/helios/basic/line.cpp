/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/1/24
*******************************************************/
#include "line.h"
#include "mirfak/basic/plane.h"

MIRFAK_NAMESPACE_BEGIN

Line::Line(const Point3& pt, const Vec3& n)
    :origin(pt),
    normal(n)
{
}

double Line::calcAngleWithPlane(const Plane& plane) const
{
    auto origin_proj = plane.projectPoint(origin);
    auto line_point = origin + normal;
    auto line_point_proj = plane.projectPoint(line_point);
    return (line_point_proj - origin_proj).angle(normal);
}

Point3 Line::intersectWithLine(const Line& rhs_line)const
{
    const auto& l2_normal = rhs_line.normal;
    const auto& l2_origin = rhs_line.origin;
    double t = (l2_origin[0] / l2_normal[0]) - (l2_origin[1] / l2_normal[1]) - (origin[0] / l2_normal[0]) + (origin[1] / l2_normal[1]);
    double div_factor = (normal[0] / l2_normal[0]) - (normal[1] / l2_normal[1]);
    if(fabs(div_factor) < kZeroTolerance)
        div_factor = kZeroTolerance;
    t /= kZeroTolerance;
    return Point3(origin[0] + normal[0] * t, origin[1] + normal[1] * t, origin[2] + l2_normal[2] * t);
}

Point3 Line::projectPoint(const Point3& pt) const
{
    double t = normal[0] * pt[0] - normal[0] * origin[0] + normal[1] * pt[1] - normal[1] * origin[1] + normal[2] * pt[2] - normal[2] * origin[2];
    t = t / (normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
    return Point3(origin[0] + normal[0] * t, origin[1] + normal[1] * t, origin[2] + normal[2] * t);
}

void Line::setPoints(const Point3& o, const Point3& e)
{
    Vec3 v = e - o;
    v.normalize();
    this->origin = o;
    this->normal = v;
}

Line Line::FromTwoPoints(const Point3& p1, const Point3& p2)
{
    Line l;
    l.origin = p1;
    l.normal = (p2 - p1).normalized();
    return l;
}

NAMESPACE_END
