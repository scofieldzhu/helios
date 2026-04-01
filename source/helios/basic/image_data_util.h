/******************************************************** 
* author: scofieldzhu
* time:2025/6/17
*******************************************************/
#ifndef __image_data_util_h__
#define __image_data_util_h__

#include <tuple>
#include "helios/basic/helios_basic_typedef.h"
#include "helios/basic/helios_basic_export.h"

class vtkImageData;

HELIOS_NAMESPACE_BEGIN

HELIOS_BASIC_API Point3 ImageToWorld(vtkImageData* data, const Point3i& image_point);
HELIOS_BASIC_API Point3i WorldToImage(vtkImageData* data, const Point3& world_point);
HELIOS_BASIC_API bool IsInImage(vtkImageData* data, const Point3& world_point);

HELIOS_BASIC_API Arry6d CalcDataBounds(vtkImageData* data);

using ImageCornerPoints = std::array<Point3, 8>;
HELIOS_BASIC_API ImageCornerPoints CalcBoundCornerPoints(vtkImageData* data);

HELIOS_BASIC_API std::tuple<double, Point3> CalcPushRangeAndStartPointOfImagePlane(vtkImageData* data, const Vec3& plane_normal);

NAMESPACE_END

#endif
