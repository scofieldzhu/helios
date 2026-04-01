/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/3/14
*******************************************************/
#include "slice_plane_sprite.h"
#include <format>
#include <vtkPlaneSource.h>
#include <vtkPlane.h>
#include <vtkImageMapToColors.h>
#include <vtkTextureMapToPlane.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkImageReslice.h>
#include <vtkTransform.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkTexture.h>
#include <vtkCellPicker.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkImageClip.h>
#include <vtkRenderer.h>
#include <vtkProp3DCollection.h>
#include "helios/basic/log_service.h"
#include "helios/basic/coord_conv.h"
#include "helios/basic/plane.h"
#include "helios/basic/image_data_util.h"
#include "scene.h"

HELIOS_NAMESPACE_BEGIN

namespace {
	inline std::string GetPlaneActorName(std::size_t input_index)
	{
		return std::format("SlicePlane_{}", input_index);
	}
}

SlicePlaneSprite::SlicePlaneSprite()
	:plane_source_(vtkSmartPointer<vtkPlaneSource>::New()),
	reslice_axes_(vtkSmartPointer<vtkMatrix4x4>::New()),
	plane_equation_(vtkSmartPointer<vtkPlane>::New())
{
	plane_source_->SetXResolution(1);
	plane_source_->SetYResolution(1);
}

SlicePlaneSprite::~SlicePlaneSprite()
{
	//removeAllInputs();
}

bool SlicePlaneSprite::hasChangedSince(unsigned long since_mtime)const
{
	if(Sprite::hasChangedSince(since_mtime)){
		return true;
	}
	for(auto i = 0; i < process_chains_.size(); i++){
		if(process_chains_[i].input->GetMTime() <= since_mtime){
			continue;
		}
		// if(planeType() == OrthoPlaneType::kCustom) {
		// 	updateNormal();
		// 	updateOrigin();
		// }
		return true;
	}
	for(auto i = 0; i < getNumberOfInputs(); i++) {
		auto table = getLookupTable(i);
		if(table != nullptr && table->GetMTime() > since_mtime){
			return true;
		}
	}
	return false;
}

PickInfoList SlicePlaneSprite::getPickList(vtkCellPicker* picker, Scene& scene, int x, int y)const
{	
	if(!(getVisibility(0, &scene) && picker)){
		return {};
	}
	PickInfoList result_info_list;
	auto actors = getSceneActors(&scene);
	Point3 origin = plane_source_->GetOrigin();
	Point3 p2 = plane_source_->GetPoint2();
	Vec3 vec = p2 - origin;
	actors->InitTraversal();
	auto actor = actors->GetNextActor();
	while(actor){
		auto pos = picker->GetProp3Ds()->IsItemPresent(actor);
		if(pos == 0){
            actor = actors->GetNextActor();
			continue;
		}
		PickInfo info;
		info.prop = actor;
		info.position = picker->GetPickedPositions()->GetPoint(pos - 1);
		info.normal = getTransform()->TransformNormal(plane_source_->GetNormal());
		info.vector = getTransform()->TransformVector(vec);
		info.sprite = const_cast<Sprite*>((const Sprite*)this);
		info.scene = &scene;
		result_info_list.push_back(std::move(info));
		actor = actors->GetNextActor();
	}
	for(auto child : children()){
		PickInfoList infos = child->getPickList(picker, scene, x, y);
		if(!infos.empty()){
			std::copy(infos.begin(), infos.end(), std::back_inserter(result_info_list));
		}
	}
	return result_info_list;
}

void SlicePlaneSprite::makeActors(Scene& scene)
{
	for(auto index = 0; index < getNumberOfInputs(); index++){		
		auto texture_off = isTextureOffScene(&scene);
		auto actor = makePlaneActor(index, texture_off);
		addSceneProp(scene, actor, GetPlaneActorName(index));
	}
}

vtkActor* SlicePlaneSprite::makePlaneActor(std::size_t input_index, bool no_texture)
{
	auto actor = vtkActor::New();
	vtkNew<vtkDataSetMapper> mapper;
	if(!no_texture){
		mapper->SetInputConnection(process_chains_[input_index].texture_to_plane_mapper->GetOutputPort());
		vtkNew<vtkTexture> texture;
		texture->SetQualityTo32Bit();
		texture->SetColorModeToDefault();
		texture->SetInputConnection(process_chains_[input_index].image_to_colors_mapper->GetOutputPort());
		texture->SetInterpolate(texture_interpolate_);
		texture->RepeatOff();
		auto lookuptable = getLookupTable(input_index);
		if(lookuptable != nullptr){
			texture->SetLookupTable(lookuptable);
		}
		actor->SetTexture(texture);
	}else{
		mapper->SetInputConnection(plane_source_->GetOutputPort());
	}
	actor->SetMapper(mapper);
	actor->SetVisibility(process_chains_[input_index].visibility);
	return actor;
}

void SlicePlaneSprite::addPlaneActor(std::size_t input_index)
{
	if(input_index < process_chains_.size()){
		for(auto scene : getDisplaySceneList()){
			auto actor = makePlaneActor(input_index, isTextureOffScene(scene));
			addSceneProp(*scene, actor, GetPlaneActorName(input_index));
			actor->SetPickable(input_index == 0);
		}
	}
}

void SlicePlaneSprite::removeSlicePlaneActor(std::size_t input_index)
{
	for(auto scene : getDisplaySceneList()){
		auto actor_name = GetPlaneActorName(input_index);
		auto plane_actor = getSceneActor(actor_name, scene);
		if(plane_actor == nullptr){
			continue;
		}
		removeSceneProp(*scene, plane_actor);
		auto collection = getSceneActors(scene);
		if(input_index == 0 && collection->GetNumberOfItems() > 1){
			collection->InitTraversal();
			auto first_actor = collection->GetNextActor();
			auto second_actor = collection->GetNextActor();
			second_actor->SetPickable(first_actor->GetPickable());
		}
	}
}

void SlicePlaneSprite::replaceLookupTable(vtkLookupTable* lookup_table, std::size_t input_index)
{
	for(Scene* scene : getDisplaySceneList()) {
		auto plane_actor = getSceneActor(GetPlaneActorName(input_index), scene);
		if(plane_actor && plane_actor->GetTexture()){
			plane_actor->GetTexture()->SetLookupTable(lookup_table);
		}
	}
}

void SlicePlaneSprite::setPlaneActorVisibility(const Scene* target_scene, std::size_t input_index, bool isvisible)
{
	if(target_scene){
		auto plane_actor = getSceneActor(GetPlaneActorName(input_index), target_scene);
		if(plane_actor){
			plane_actor->SetVisibility(isvisible);
		}
	}
}

bool SlicePlaneSprite::getPlaneActorVisibility(const Scene& target_scene, std::size_t input_index)const
{
	auto plane_actor = getSceneActor(GetPlaneActorName(input_index), &target_scene);
	return plane_actor && plane_actor->GetVisibility();
}

void SlicePlaneSprite::changeSliecPlaneTextureInterpolate(bool yes_no)
{
	auto actor_collection = getSceneActors(nullptr);
	actor_collection->InitTraversal();
	auto cur_actor = actor_collection->GetNextActor();
	while(cur_actor){
		auto texture = cur_actor->GetTexture();
		if(texture){	
			texture->SetInterpolate(yes_no);
		}
		cur_actor = actor_collection->GetNextActor();
	}
}

std::size_t SlicePlaneSprite::addInputData(vtkImageData* input)
{
	if(input == nullptr){
		return process_chains_.size();
	}
    InputProcessChain process_chain;
    process_chain.input = input;

    vtkNew<vtkTransform> reslicer_trans;
	reslicer_trans->SetInput(getTransform());
	vtkNew<vtkMatrixToLinearTransform> axes_trans;
	axes_trans->SetInput(reslice_axes_);
	reslicer_trans->Concatenate(axes_trans);
	vtkNew<vtkImageReslice> reslicer;
	reslicer->SetEnableSMP(isSMPEnabled());
	reslicer->SetInputData(input);
	reslicer->SetResliceTransform(reslicer_trans);
	reslicer->SetInterpolationMode(reslice_interpolate_ ? VTK_RESLICE_LINEAR : VTK_RESLICE_NEAREST);
	reslicer->SetBackgroundLevel(input->GetScalarRange()[0]);
	reslicer->OptimizationOn();
    process_chain.reslicer = reslicer;

    vtkNew<vtkImageMapToColors> colors;
	colors->SetEnableSMP(isSMPEnabled());
	colors->SetInputConnection(reslicer->GetOutputPort());
	colors->PassAlphaToOutputOn();
    process_chain.image_to_colors_mapper = colors;

	vtkNew<vtkTextureMapToPlane> coords;
	coords->SetInputConnection(plane_source_->GetOutputPort());
	coords->AutomaticPlaneGenerationOff();
    process_chain.texture_to_plane_mapper = coords;

    process_chains_.emplace_back(std::move(process_chain));

	addPlaneActor(process_chains_.size() - 1);
	updateOutputFormatInfo(process_chains_.size() - 1, nullptr);
	updateNormal();
	updateOrigin();    
	return process_chains_.size();
}

vtkImageData* SlicePlaneSprite::getInputData(std::size_t i)const
{
	return i < process_chains_.size() ? process_chains_[i].input : nullptr;
}

void SlicePlaneSprite::setInputData(vtkImageData* input, std::size_t i)
{
	if(i >= process_chains_.size() || input == nullptr){
		SPDLOG_WARN("Invalid argument input index:{} or null data object!");
		return;
	}
	process_chains_[i].input = input;
	process_chains_[i].reslicer->SetInputData(input);
	process_chains_[i].reslicer->GetInputAlgorithm()->Modified();
	process_chains_[i].reslicer->SetBackgroundLevel(input->GetScalarRange()[0]);
	updateOutputFormatInfo(i, vtkLookupTable::SafeDownCast(process_chains_[i].image_to_colors_mapper->GetLookupTable()));
	updateNormal();
	updateOrigin();
	modified();
}

void SlicePlaneSprite::removeInputData(std::size_t i)
{
	if(i >= process_chains_.size()){
		return;
	}
	removeSlicePlaneActor(i);
    process_chains_.erase(process_chains_.begin() + i);
	modified();
}

vtkSmartPointer<vtkPlaneSource> SlicePlaneSprite::getPlaneSource() const
{
    return plane_source_;
}

void SlicePlaneSprite::updateOutputFormatInfo(std::size_t clr_index, vtkLookupTable* lookuptable)
{
	if(clr_index >= process_chains_.size()){
		return;
	}
	auto reslicer = process_chains_[clr_index].reslicer; 
	reslicer->UpdateInformation();
	auto resliced_data = vtkImageData::SafeDownCast(reslicer->GetOutputDataObject(0));
	if(resliced_data == nullptr){
		return;
	}
    auto clr_mapper = process_chains_[clr_index].image_to_colors_mapper;
	switch(resliced_data->GetNumberOfScalarComponents()){
		case 3:
			clr_mapper->SetOutputFormatToRGB();
			clr_mapper->SetLookupTable(nullptr);
			break;
		case 4:
			clr_mapper->SetOutputFormatToRGBA();
			clr_mapper->SetLookupTable(nullptr);
			break;
		default:
			clr_mapper->SetLookupTable(lookuptable);
			if(clr_index == 0){
				clr_mapper->SetOutputFormatToRGBA();
			}
			break;
	}
}

vtkSmartPointer<vtkPlane> SlicePlaneSprite::getPlaneEquation()const
{
	plane_equation_->SetNormal(plane_source_->GetNormal());
	plane_equation_->SetOrigin(plane_source_->GetOrigin());
	return plane_equation_;
}

void SlicePlaneSprite::setPosition(double position)
{
	if(pushRange() <= 0.0){
		return;
	}		
	position = std::clamp(position, 0.0, pushRange());
	Vec3 vec = getNormal().normalized();
	auto distance = Plane(start_point_, vec).getProjectDistance(getOrigin());
	push(position - distance);
	modified();
}

void SlicePlaneSprite::updatePosition()
{
	Vec3 vec = getNormal().normalized();
	auto distance = Plane(start_point_, vec).getProjectDistance(getOrigin());
    PositionChangeSignal.invoke(this, distance);
}

double SlicePlaneSprite::getPosition() const
{
    Vec3 vec = getNormal().normalized();
    return Plane(start_point_, vec).getProjectDistance(getOrigin());
}

Point3 SlicePlaneSprite::projectPoint(const Point3& pos, double offset)
{
	Point3 p2 = getNormal() + pos;
	auto target_pt = intersectWithLine(pos, p2);
	return target_pt + getNormal() * offset;
}

void SlicePlaneSprite::updateNormal()
{
	plane_equation_->SetNormal(plane_source_->GetNormal());
	plane_equation_->SetOrigin(plane_source_->GetOrigin());	
	Point3 plane_origin = plane_source_->GetOrigin();
	Point3 plane_point1 = plane_source_->GetPoint1();
	Point3 plane_point2 = plane_source_->GetPoint2();
	Vec3 plane_axis1 = plane_point1 - plane_origin;
	Vec3 plane_axis2 = plane_point2 - plane_origin;
	const double kPlaneSizeX = plane_axis1.norm();
	const double kPlaneSizeY = plane_axis2.norm();
	if(IsZero(kPlaneSizeX) || IsZero(kPlaneSizeY)){
		return;
	}
	plane_axis1 /= kPlaneSizeX;
	plane_axis2 /= kPlaneSizeY;
	reslice_axes_->SetElement(0, 0, plane_axis1[0]);
	reslice_axes_->SetElement(1, 0, plane_axis1[1]);
	reslice_axes_->SetElement(2, 0, plane_axis1[2]);
	reslice_axes_->SetElement(3, 0, 0.0);
	reslice_axes_->SetElement(0, 1, plane_axis2[0]);
	reslice_axes_->SetElement(1, 1, plane_axis2[1]);
	reslice_axes_->SetElement(2, 1, plane_axis2[2]);
	reslice_axes_->SetElement(3, 1, 0.0);
	Vec3 plane_normal = plane_source_->GetNormal();
	reslice_axes_->SetElement(0, 2, plane_normal[0]);
	reslice_axes_->SetElement(1, 2, plane_normal[1]);
	reslice_axes_->SetElement(2, 2, plane_normal[2]);
	reslice_axes_->SetElement(3, 2, 0.0);
	reslice_axes_->SetElement(0, 3, 0);
	reslice_axes_->SetElement(1, 3, 0);
	reslice_axes_->SetElement(2, 3, 0);
	reslice_axes_->SetElement(3, 3, 1.0);
	reslice_axes_->Transpose(); // equal to "Inverse". make world to local coordinate

	Arry4d origin = {0.0};
	Arry4d point = {plane_origin[0], plane_origin[1], plane_origin[2], 1.0};
	reslice_axes_->MultiplyPoint(point.data(), origin.data());
	reslice_axes_->Transpose(); // local to world coordinate
	Arry4d point1 = {0.0, 0.0, origin[2], 1.0};
	Arry4d new_origin = {0.0};
	reslice_axes_->MultiplyPoint(point1.data(), new_origin.data());
	reslice_axes_->SetElement(0, 3, new_origin[0]);
	reslice_axes_->SetElement(1, 3, new_origin[1]);
	reslice_axes_->SetElement(2, 3, new_origin[2]);
	for(auto i = 0; i < getNumberOfInputs(); i++) {		
		auto spacing = process_chains_[i].input->GetSpacing();
		double dst_spacing_x = abs(plane_axis1[0] * spacing[0]);
		dst_spacing_x += abs(plane_axis1[1] * spacing[1]);
		dst_spacing_x += abs(plane_axis1[2] * spacing[2]);
		double dst_spacing_y = abs(plane_axis2[0] * spacing[0]);
		dst_spacing_y += abs(plane_axis2[1] * spacing[1]);
		dst_spacing_y += abs(plane_axis2[2] * spacing[2]);
		if(IsZero(dst_spacing_x) || IsZero(dst_spacing_y)){
			return;
		}
		int dst_extent_x = 1;
		while(dst_extent_x < kPlaneSizeX / dst_spacing_x){
			dst_extent_x = dst_extent_x << 1;
		}
		int dst_extent_y = 1;
		while(dst_extent_y < kPlaneSizeY / dst_spacing_y){
			dst_extent_y = dst_extent_y << 1;
		}	
        auto reslicer = process_chains_[i].reslicer;
		reslicer->SetOutputSpacing(dst_spacing_x, dst_spacing_y, 1);
		reslicer->SetOutputOrigin(0.5 * dst_spacing_x + origin[0], 0.5 * dst_spacing_y + origin[1], 0.0);
		reslicer->SetOutputExtent(0, dst_extent_x - 1, 0, dst_extent_y - 1, 0, 0);
		double expand1 = dst_extent_x * dst_spacing_x;
        auto coords_mapper = process_chains_[i].texture_to_plane_mapper;
		coords_mapper->SetOrigin(plane_origin[0], plane_origin[1], plane_origin[2]);
		coords_mapper->SetPoint1(
			plane_origin[0] + plane_axis1[0] * expand1, 
			plane_origin[1] + plane_axis1[1] * expand1, 
			plane_origin[2] + plane_axis1[2] * expand1
		);
		double expand2 = dst_extent_y * dst_spacing_y;
		coords_mapper->SetPoint2(
			plane_origin[0] + plane_axis2[0] * expand2,
			plane_origin[1] + plane_axis2[1] * expand2, 
			plane_origin[2] + plane_axis2[2] * expand2
		);
	}
	modified();
}

void SlicePlaneSprite::updateOrigin()
{
	if(restrict_to_volume_){
		Arry6d vbounds = {0.0};
		if(Point3(vol_bounds_[0], vol_bounds_[1], vol_bounds_[2]) != Point3(vbounds[0], vbounds[1], vbounds[2]) ||
		   Point3(vol_bounds_[3], vol_bounds_[4], vol_bounds_[5]) != Point3(vbounds[3], vbounds[4], vbounds[5])){ // volume range changed!
			vbounds = CalcDataBounds(getInputData());
		}
		Vec3 normal = plane_source_->GetNormal();
		Vec3 abs_normal = {abs(normal[0]), abs(normal[1]), abs(normal[2])};
		Point3 center = plane_source_->GetCenter();
		//calculate which axis direction should be confirmed as "major axis"!
		std::size_t i = 0;
		if(abs_normal[0] >= abs_normal[1] && abs_normal[0] >= abs_normal[2]){
			i = 0;
		}
		if(abs_normal[1] >= abs_normal[0] && abs_normal[1] >= abs_normal[2]){
			i = 1;
		}
		if(abs_normal[2] >= abs_normal[0] && abs_normal[2] >= abs_normal[1]){
			i = 2;
		}
		//clamp center to volume range!
		if(center[i] > vbounds[2 * i + 1]){
			center[i] = vbounds[2 * i + 1];
			plane_source_->SetCenter(center[0], center[1], center[2]);
		}else if(center[i] < vbounds[2 * i]){
			center[i] = vbounds[2 * i];
			plane_source_->SetCenter(center[0], center[1], center[2]);
		}
	}
	plane_equation_->SetOrigin(plane_source_->GetOrigin());

	Arry4d plane_origin = {0};
	plane_source_->GetOrigin(plane_origin.data());
	plane_origin[3] = 1.0;
	reslice_axes_->SetElement(0, 3, 0);
	reslice_axes_->SetElement(1, 3, 0);
	reslice_axes_->SetElement(2, 3, 0);
	reslice_axes_->Transpose(); // convert world to local coordinate
	Arry4d origin = {0};
	reslice_axes_->MultiplyPoint(plane_origin.data(), origin.data());
	reslice_axes_->Transpose();
	Arry4d temp = {0.0, 0.0, origin[2], 1.0};
	Arry4d new_origin = {0};
	reslice_axes_->MultiplyPoint(temp.data(), new_origin.data());
	reslice_axes_->SetElement(0, 3, new_origin[0]);
	reslice_axes_->SetElement(1, 3, new_origin[1]);
	reslice_axes_->SetElement(2, 3, new_origin[2]);
	for(auto i = 0; i < getNumberOfInputs(); i++){
		auto spacing = process_chains_[i].reslicer->GetOutputSpacing();
		process_chains_[i].reslicer->SetOutputOrigin(0.5 * spacing[0] + origin[0], 0.5 * spacing[1] + origin[1], 0.0);
	}
	modified();
}

Point3 SlicePlaneSprite::intersectWithViewRay(double x, double y, const Scene& scene)
{
	Point3 p1 = scene.displayToWorld({x, y, 0.25});
	Point3 p2 = scene.displayToWorld({x, y, 0.75});
	return intersectWithLine(p1, p2);
}

Point3 SlicePlaneSprite::intersectWithLine(const Point3& p1, const Point3& p2)
{
	Vec3 n = getNormal();
	const Vec3 transformed_normal = getTransform()->TransformNormal(n[0], n[1], n[2]);
	Point3 c = getCenter();
	const Point3 transformed_center = getTransform()->TransformNormal(c[0], c[1], c[2]);
	if(transformed_normal[0] == 0.0 && p1[1] == p2[1] && p1[2] == p2[2] && plane_type_ == kAxial){
		return {c[0], p1[1], p1[2]};
	}		
	if(transformed_normal[1] == 0.0 && p1[0] == p2[0] && p1[2] == p2[2] && plane_type_ == kSagittal){
		return {p1[0], c[1], p1[2]};
	}		
	if(transformed_normal[2] == 0.0 && p1[0] == p2[0] && p1[1] == p2[1] && plane_type_ == kCoronal){
		return {p1[0], p1[1], c[2]};
	}
	Vec3 l = p2 - p1;
	double dot_prod = transformed_normal.dot(l);
	Vec3 v = transformed_center - p1;
	double t = transformed_normal.dot(v) / dot_prod;
	l *= t;
	return p1 + l;
}

void SlicePlaneSprite::setOpacity(const Scene* scene, double alpha, std::size_t index)
{
	if(index < process_chains_.size()){
		auto actor = getSceneActor(GetPlaneActorName(index), scene);
		if(actor){
			actor->GetProperty()->SetOpacity(alpha);
			modified();
		}
	}
}

double SlicePlaneSprite::getOpacity(const Scene* scene, std::size_t index)const
{
	if(index < process_chains_.size() && checkConnective(*scene)) {
		auto actor = getSceneActor(GetPlaneActorName(index), scene);
		if(actor){
			return actor->GetProperty()->GetOpacity();
		}
	}
	return 1.0;
}

void SlicePlaneSprite::setVisibility(bool yes_no, std::size_t index, const Scene* scene)
{
	if(process_chains_.empty() || index >= process_chains_.size()){
		return;
	}
	if(scene == nullptr){
		process_chains_[index].visibility = yes_no;
	}
	setPlaneActorVisibility(scene, index, yes_no);
	modified();
}

bool SlicePlaneSprite::getVisibility(std::size_t index, const Scene* scene)const
{
	if(index >= process_chains_.size())
		return false;
	if(scene == nullptr){
		return process_chains_[index].visibility;
	}
	return getPlaneActorVisibility(*scene, index);
}

void SlicePlaneSprite::setPlaneType(Type plane_type)
{
	if(process_chains_.empty()) {
		SPDLOG_ERROR("No input data found!");
		return;
	}
	plane_type_ = plane_type;
	auto input = process_chains_[0].input;
	auto ext = input->GetExtent();
	Arry6i extent = {*ext, *(ext + 1), *(ext + 2), *(ext + 3), *(ext + 4), *(ext + 5) };
	Point3 origin = input->GetOrigin();
	auto spc = input->GetSpacing();
	Arry3d spacing = {*spc, *(spc + 1), *(spc + 2) };
	double bound_x_min = origin[0] + spacing[0] * (extent[0] - 0.5);
	double bound_x_max = origin[0] + spacing[0] * (extent[1] + 0.5);
	Arry2d xbounds = {bound_x_min, bound_x_max};
	double bound_y_min = origin[1] + spacing[1] * (extent[2] - 0.5);
	double bound_y_max = origin[1] + spacing[1] * (extent[3] + 0.5);
	Arry2d ybounds = {bound_y_min, bound_y_max};
	double bound_z_min = origin[2] + spacing[2] * (extent[4] - 0.5);
	double bound_z_max = origin[2] + spacing[2] * (extent[5] + 0.5);
	Arry2d zbounds = {bound_z_min, bound_z_max};
	vol_bounds_ = CalcDataBounds(input);
	plane_source_->SetXResolution(1);
	plane_source_->SetYResolution(1);
	switch(plane_type) {
		case kAxial:
			plane_source_->SetOrigin(xbounds[0], ybounds[0], 0);
			plane_source_->SetPoint1(xbounds[1], ybounds[0], 0);
			plane_source_->SetPoint2(xbounds[0], ybounds[1], 0);
			break;
		case kSagittal:
			plane_source_->SetOrigin(0, ybounds[0], zbounds[0]);
			plane_source_->SetPoint1(0, ybounds[1], zbounds[0]);
			plane_source_->SetPoint2(0, ybounds[0], zbounds[1]);
			break;
		case kCoronal:
			plane_source_->SetOrigin(xbounds[0], 0, zbounds[0]);
			plane_source_->SetPoint1(xbounds[0], 0, zbounds[1]);
			plane_source_->SetPoint2(xbounds[1], 0, zbounds[0]);
			//plane_source_->SetPoint1(xbounds[1], 0, zbounds[0]);
			//plane_source_->SetPoint2(xbounds[0], 0, zbounds[1]);
			break;
		default: //kCustom
			break;
	}
	updateNormal();
	updateOrigin();
}

void SlicePlaneSprite::setSMPEnabled(bool enabled)
{
	smp_enabled_ = enabled;
	modified();
}

double SlicePlaneSprite::push(double request_dist)
{
    if(pushRange() <= 0.0){
        return 0.0;
    }
	Point3 o1 = getTransformedCenter();
	plane_source_->Push(request_dist);
	updateOrigin();
	Point3 o2 = getTransformedCenter();
	Vec3 n = getTransformedNormal();
    updatePosition();
	return Vec3(o2 - o1).dot(n);
}

void SlicePlaneSprite::setPushRange(double range)
{
	push_range_ = range;
    PushRangeChangeSignal.invoke(this, push_range_);
	modified();
}

void SlicePlaneSprite::setNormal(const Vec3& normal)
{
	plane_source_->SetNormal(normal[0], normal[1], normal[2]);
	updateNormal();
	modified();
}

Vec3 SlicePlaneSprite::getNormal()const
{
	return plane_source_->GetNormal();
}

double SlicePlaneSprite::getSize1()const
{
	Point3 o = plane_source_->GetOrigin();
	Point3 p1 = plane_source_->GetPoint1();
	return Vec3(p1 - o).norm();
}

double SlicePlaneSprite::getSize2()const
{
	Point3 o = plane_source_->GetOrigin();
	Point3 p2 = plane_source_->GetPoint2();
	return (p2 - o).norm();
}

void SlicePlaneSprite::setOrigin(const Point3& origin)
{
	plane_source_->SetOrigin(origin[0], origin[1], origin[2]);
	updateOrigin();
	modified();
}

Point3 SlicePlaneSprite::getOrigin()const
{
	return plane_source_->GetOrigin();
}

void SlicePlaneSprite::setPoint1(const Point3& p1)
{
	plane_source_->SetPoint1(p1[0], p1[1], p1[2]);
	updateNormal();
	modified();
}

Point3 SlicePlaneSprite::getPoint1() const
{
	return plane_source_->GetPoint1();
}

void SlicePlaneSprite::setPoint2(const Point3& p2)
{
	plane_source_->SetPoint2(p2[0], p2[1], p2[2]);
	updateNormal();
	modified();
}

Point3 SlicePlaneSprite::getPoint2() const
{
	return plane_source_->GetPoint2();
}

void SlicePlaneSprite::setCenter(const Point3& center)
{
	plane_source_->SetCenter(center[0], center[1], center[2]);
	updateOrigin();
	modified();
}

Point3 SlicePlaneSprite::getCenter() const
{
	return plane_source_->GetCenter();
}

Point3 SlicePlaneSprite::calcCenter() const
{
    auto w1 = getSize1();
    auto w2 = getSize2();
    Point3 o = getOrigin();
    Point3 p1 = getPoint1();
    Vec3 v1 = (p1 - o).normalized();
    Point3 p2 = getPoint2();
    Vec3 v2 = (p2 - o).normalized();
    Point3 c = o + v1 * (w1 / 2.0);
    c += v2 * (w2 / 2.0);
    return c;
}

void SlicePlaneSprite::setStartPoint(const Point3& pt)
{
	start_point_ = pt;
}

void SlicePlaneSprite::setLookupTable(vtkLookupTable* table, std::size_t i)
{
	if(i < process_chains_.size() && table){
		updateOutputFormatInfo(i, table);
		replaceLookupTable(table, i);
		modified();
	}
}

vtkLookupTable* SlicePlaneSprite::getLookupTable(std::size_t index)const
{
	return index < process_chains_.size() ? vtkLookupTable::SafeDownCast(process_chains_[index].image_to_colors_mapper->GetLookupTable()) : nullptr;
}

void SlicePlaneSprite::addTextureOffScene(Scene* scene)
{
	if(!isTextureOffScene(scene)){
		texture_off_scenes_.push_back(scene);
	}
}

bool SlicePlaneSprite::isTextureOffScene(Scene* scene) const
{
	return std::find(texture_off_scenes_.begin(), texture_off_scenes_.end(), scene) != texture_off_scenes_.end();
}

void SlicePlaneSprite::setRestrictToVolume(bool yesno)
{
	restrict_to_volume_ = yesno;
	modified();
}

void SlicePlaneSprite::setResliceInterpolate(bool yes_no)
{
	reslice_interpolate_ = yes_no;
	for(auto& pc : process_chains_){
        pc.reslicer->SetInterpolationMode(yes_no ? VTK_RESLICE_LINEAR : VTK_RESLICE_NEAREST);
	}
	modified();
}

void SlicePlaneSprite::setTextureInterpolate(bool yes_no)
{
	texture_interpolate_ = yes_no;
	changeSliecPlaneTextureInterpolate(yes_no);
	modified();
}

Point3 SlicePlaneSprite::getTransformedCenter()const
{
	return getTransform()->TransformPoint(getCenter());
}

Vec3 SlicePlaneSprite::getTransformedNormal()const
{
	return getTransform()->TransformNormal(getNormal());
}

vtkPolyData* SlicePlaneSprite::getPolyData()
{
	return plane_source_->GetOutput();
}

Vec3 SlicePlaneSprite::getTransformedViewUp2Vector()const
{
	return getTransform()->TransformVector(getViewUp2Vector());
}

Vec3 SlicePlaneSprite::getViewUp2Vector()const
{
	Point3 origin = plane_source_->GetOrigin();
	Point3 p2 = plane_source_->GetPoint2();
	Vec3 vec = p2 - origin;
	vec.normalize();
	return vec;
}

Vec3 SlicePlaneSprite::getTransformedViewUp1Vector()const
{
	return getTransform()->TransformVector(getViewUp1Vector());
}

Vec3 SlicePlaneSprite::getViewUp1Vector()const
{
	Point3 origin = plane_source_->GetOrigin();
	Point3 p1 = plane_source_->GetPoint1();
	Vec3 vec = p1 - origin;
	vec.normalize();
	return vec;
}

void SlicePlaneSprite::reset()
{
	removeAllInputs();
	plane_source_ = vtkSmartPointer<vtkPlaneSource>::New();
	plane_source_->SetXResolution(1);
	plane_source_->SetYResolution(1);
	reslice_axes_ = vtkSmartPointer<vtkMatrix4x4>::New();
	plane_type_ = kCustom;
	reslice_interpolate_ = true;
	texture_interpolate_ = true;
	restrict_to_volume_ = false;
	push_range_ = 0.0;
	vol_bounds_ = {0.0};
	start_point_ = {0.0, 0.0, 0.0};
}

void SlicePlaneSprite::removeAllInputs()
{
	//make sure it's not already connected    
	//logverify(visualizerlogger, !existsDisplayPane());
	process_chains_.clear();
	plane_source_ = nullptr;
	reslice_axes_ = nullptr;
}

NAMESPACE_END
