/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/6/17
*******************************************************/
#ifndef __image_data_util_h__
#define __image_data_util_h__

#include <tuple>
#include "mirfak/basic/mirfak_basic_typedef.h"
#include "mirfak/basic/mirfak_basic_export.h"

class vtkImageData;

MIRFAK_NAMESPACE_BEGIN

MIRFAK_BASIC_API Point3 ImageToWorld(vtkImageData* data, const Point3i& image_point);
MIRFAK_BASIC_API Point3i WorldToImage(vtkImageData* data, const Point3& world_point);
MIRFAK_BASIC_API bool IsInImage(vtkImageData* data, const Point3& world_point);

MIRFAK_BASIC_API Arry6d CalcDataBounds(vtkImageData* data);

using ImageCornerPoints = std::array<Point3, 8>;
MIRFAK_BASIC_API ImageCornerPoints CalcBoundCornerPoints(vtkImageData* data);

MIRFAK_BASIC_API std::tuple<double, Point3> CalcPushRangeAndStartPointOfImagePlane(vtkImageData* data, const Vec3& plane_normal);

NAMESPACE_END

#endif
