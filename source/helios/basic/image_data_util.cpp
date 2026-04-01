/******************************************************** 
* author: scofieldzhu
* time:2025/6/17
*******************************************************/
#include "image_data_util.h"
#include <cassert>
#include <vtkImageData.h>
#include "helios/basic/line.h"

#include <vtkMetaImageReader.h>

HELIOS_NAMESPACE_BEGIN

Point3 ImageToWorld(vtkImageData* data, const Point3i& image_point)
{
    if(data == nullptr){
        return {};
    }
    auto extents = data->GetExtent();
    if(image_point[0] < extents[0] || image_point[0] > extents[1]){
        return {};
    }
    if(image_point[1] < extents[2] || image_point[1] > extents[3]){
        return {};
    }
    if(image_point[2] < extents[4] || image_point[2] > extents[5]){
        return {};
    }
    auto origin = data->GetOrigin();
    auto spacing = data->GetSpacing();
    return Point3(origin[0] + image_point[0] * spacing[0], origin[1] + image_point[1] * spacing[1], origin[2] + image_point[2] * spacing[2]);
}

Point3i WorldToImage(vtkImageData* data, const Point3& world_point)
{
    if(data == nullptr){
        return {};
    }
    auto bounds = data->GetBounds();
    auto spacing = data->GetSpacing();
    Point3i image_point;
    image_point[0] = static_cast<int>(0.5 + (world_point[0] - bounds[0]) / spacing[0]); 
    image_point[1] = static_cast<int>(0.5 + (world_point[1] - bounds[2]) / spacing[1]);
    image_point[2] = static_cast<int>(0.5 + (world_point[2] - bounds[4]) / spacing[2]);
    return image_point;
}

bool IsInImage(vtkImageData* data, const Point3& world_point)
{
    if(data == nullptr){
        return false;
    }
    auto bounds = data->GetBounds();
    if(world_point[0] < bounds[0] || world_point[0] > bounds[1])
        return false;
    if(world_point[1] < bounds[2] || world_point[1] > bounds[3])
        return false;
    if(world_point[2] < bounds[4] || world_point[2] > bounds[5])
        return false;
    return true;
}

Arry6d CalcDataBounds(vtkImageData* data)
{
    Arry6d result_bounds{0.0};
    if(data == nullptr){
        return result_bounds;
    }
    auto exts = data->GetExtent();
    auto spacings = data->GetSpacing();
    auto origin = data->GetOrigin();
    Arry6d vbounds = {
        origin[0] + exts[0] * spacings[0],
        origin[0] + exts[1] * spacings[0],
        origin[1] + exts[2] * spacings[1],
        origin[1] + exts[3] * spacings[1],
        origin[2] + exts[4] * spacings[2],
        origin[2] + exts[5] * spacings[2]
    };
    result_bounds[0] = (std::min)(vbounds[0], vbounds[1]);
    result_bounds[1] = (std::max)(vbounds[0], vbounds[1]);
    result_bounds[2] = (std::min)(vbounds[2], vbounds[3]);
    result_bounds[3] = (std::max)(vbounds[2], vbounds[3]);
    result_bounds[4] = (std::min)(vbounds[4], vbounds[5]);
    result_bounds[5] = (std::max)(vbounds[4], vbounds[5]);
    return result_bounds;
}

ImageCornerPoints CalcBoundCornerPoints(vtkImageData* data)
{
	auto bounds = CalcDataBounds(data);
	ImageCornerPoints corner_points;
	corner_points[0] = {bounds[0], bounds[2], bounds[4]};
	corner_points[1] = {bounds[0], bounds[2], bounds[5]};
	corner_points[2] = {bounds[0], bounds[3], bounds[4]};
	corner_points[3] = {bounds[0], bounds[3], bounds[5]};
	corner_points[4] = {bounds[1], bounds[2], bounds[4]};
	corner_points[5] = {bounds[1], bounds[2], bounds[5]};
	corner_points[6] = {bounds[1], bounds[3], bounds[4]};
	corner_points[7] = {bounds[1], bounds[3], bounds[5]};
	return corner_points;
}

std::tuple<double, Point3> CalcPushRangeAndStartPointOfImagePlane(vtkImageData* data, const Vec3& plane_normal)
{
    ImageCornerPoints corner_points = CalcBoundCornerPoints(data);
    Point3 data_center = data->GetCenter();
	std::vector<double> lens;
	for(int i = 0; i < 8; ++i) {
		Point3 project_point = Line(data_center, plane_normal).projectPoint(corner_points[i]);
		double length = project_point.distanceTo(data_center);
		Vec3 vector = (project_point - data_center).normalized();
		if(vector.dot(plane_normal) < 0.0){ // check if lies on reversed direction!
            length = -length;
        }			
		lens.push_back(length);
	}
	int min_index = 0, max_index = 0;
	double min_len = lens[0], max_len = lens[0];
	for(int j = 1; j < 8; ++j){
		if(lens[j] < min_len) {
			min_len = lens[j];
			min_index = j;
		}
		if(lens[j] > max_len) {
			max_len = lens[j];
			max_index = j;
		}
	}
    Point3 start_pt = corner_points[min_index];
    double range = max_len - min_len; //must be positive
    assert(range >= 0.0);
	return std::make_tuple(range, start_pt);
}



NAMESPACE_END