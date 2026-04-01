/******************************************************** 
* author: scofieldzhu
* time:2025/1/24
*******************************************************/
#ifndef __line_h__
#define __line_h__

#include "helios/basic/helios_basic_export.h"
#include "helios/basic/xyz.hpp"

HELIOS_NAMESPACE_BEGIN

class Plane;
class HELIOS_BASIC_API Line
{
public:
    static Line FromTwoPoints(const Point3& p1, const Point3& p2);
    double calcAngleWithPlane(const Plane& plane)const;
    Point3 intersectWithLine(const Line& rhs_line)const;
    Point3 projectPoint(const Point3& pt)const;
    void setPoints(const Point3& origin, const Point3& end);
    Line& operator=(const Line&) = default;
    Line() = default;
    Line(const Point3& pt, const Vec3& n);
    Line(const Line&) = default;
    ~Line() = default;
    Point3 origin;
    Vec3 normal;
};

using LineOpt = std::optional<Line>;

NAMESPACE_END

#endif
