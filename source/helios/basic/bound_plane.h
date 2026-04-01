/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/4/18
*******************************************************/
#ifndef __bound_plane_h__
#define __bound_plane_h__

/***********************************************************
BoundPlane -represents a plane with limited lengths.
    the BoundPlane is defined by three points(origin, pt1, pt2),
and normal represents its' direction.
***********************************************************/

#include "helios/basic/helios_basic_export.h"
#include "helios/basic/xyz.hpp"

HELIOS_NAMESPACE_BEGIN

class HELIOS_BASIC_API BoundPlane
{
public:
    double length1()const;
	double length2()const;
    void setPt1(const Point3& pt);
    const Point3& pt1()const { return pt1_; }
    void setPt2(const Point3& pt);
    const Point3& pt2()const { return pt2_; }
    const Vec3 normal()const { return normal_; }
    void setOrigin(const Point3& pt);
    const Point3& origin()const { return origin_; }
    BoundPlane(const Point3& o, const Point3& p1, const Point3& p2);
    ~BoundPlane();

private:
    void updateNormal();
    Point3 pt1_, pt2_;
    Point3 origin_;
    Vec3 normal_;
};
NAMESPACE_END

#endif
