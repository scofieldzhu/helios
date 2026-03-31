/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/4/18
*******************************************************/
#include "bound_plane.h"

MIRFAK_NAMESPACE_BEGIN

BoundPlane::BoundPlane(const Point3& o, const Point3& p1, const Point3& p2)
    :origin_(o),
    pt1_(p1),
    pt2_(p2)
{
    updateNormal();
}

BoundPlane::~BoundPlane()
{}

double BoundPlane::length1() const
{
    return pt1_.distanceTo(origin_);
}

double BoundPlane::length2() const
{
    return pt2_.distanceTo(origin_);
}

void BoundPlane::setPt1(const Point3& pt)
{
    pt1_ = pt;
    updateNormal();
}

void BoundPlane::setPt2(const Point3& pt)
{
    pt2_ = pt;
    updateNormal();
}

void BoundPlane::setOrigin(const Point3& pt)
{
    origin_ = pt;
    updateNormal();
}

void BoundPlane::updateNormal()
{
    const Vec3 v1 = pt1_ - origin_;
    const Vec3 v2 = pt2_ - origin_;
    normal_ = v1.cross(v2);
    normal_.normalize();
}

NAMESPACE_END