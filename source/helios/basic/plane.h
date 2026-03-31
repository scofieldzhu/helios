/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/1/24
*******************************************************/
#ifndef __plane_h__
#define __plane_h__

#include "mirfak/basic/mirfak_basic_export.h"
#include "mirfak/basic/line.h"

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_BASIC_API Plane
{
public:
    static Pt3Opt CalcCenterOfTriplePlanes(const Plane& p1, const Plane& p2, const Plane& p3);
    LineOpt intersectWithPlane(const Plane& rhs)const;
    Pt3Opt intersectWithLine(const Line& line) const; //warning: if line is parallel with plane, point3d cannot be computed! 
    Point3 projectPoint(const Point3& pt)const;
    double getProjectDistance(const Point3& pt)const;
    Plane& operator=(const Plane&) = default;
    Plane() = default;
    Plane(const Point3& pt, const Vec3& n);
    Plane(const Plane&) = default;
    ~Plane() = default;
    Vec3 normal;
    Point3 point;
};

using PlaneOpt = std::optional<Plane>;

NAMESPACE_END

#endif