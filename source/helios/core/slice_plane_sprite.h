/******************************************************** 
* author: scofieldzhu
* time:2025/3/14
*******************************************************/

/********************************************************
SlicePlaneSprite - provides a slice view through a 3D image

The SlicePlaneSprite class provides a textured slice which cuts
through an image data set.  This class supports most of the same
methods as vtkPlaneSource.

Multiple inputs can be set, each with its own lookup table.
The opacity of the inputs can be modified in order to provide
transparency overlay views.
******************************************************************/
#ifndef __slice_plane_sprite_h__
#define __slice_plane_sprite_h__

#include "helios/core/sprite.h"

class vtkPlaneSource;
class vtkPlane;
class vtkImageMapToColors;
class vtkTextureMapToPlane;
class vtkLookupTable;
class vtkImageData;
class vtkPolyData;
class vtkImageReslice;
class vtkRenderer;
class vtkMatrix4x4;

HELIOS_NAMESPACE_BEGIN

class HELIOS_CORE_API SlicePlaneSprite : public Sprite
{
	SPRITE_DECL(SlicePlaneSprite, Sprite)
public:
    Signal<SlicePlaneSprite*, double> PositionChangeSignal;
    Signal<SlicePlaneSprite*, double> PushRangeChangeSignal;
	using Sprite::setOpacity;
	using Sprite::setVisibility;
	enum Type 
	{
		kCustom,
		kAxial,
		kCoronal,
		kSagittal
	};
	void reset();
	Point3 intersectWithViewRay(double x, double y, const Scene& scene);
	Point3 intersectWithLine(const Point3& p1, const Point3& p2);
	Point3 projectPoint(const Point3& pos, double offset = 0.0);
	PickInfoList getPickList(vtkCellPicker* picker, Scene& scene, int x, int y) const override;
	bool hasChangedSince(unsigned long since_mtime) const override;
	std::size_t getNumberOfInputs() const { return process_chains_.size(); }
	std::size_t addInputData(vtkImageData* input);
	void setInputData(vtkImageData* input, std::size_t index = 0);
	vtkImageData* getInputData(std::size_t index = 0)const;
	void removeInputData(std::size_t index);
	vtkSmartPointer<vtkPlaneSource> getPlaneSource()const;
	vtkSmartPointer<vtkPlane> getPlaneEquation()const;
	vtkPolyData* getPolyData();
	double push(double distance);
    void setStartPoint(const Point3& pt);
	Point3 startPoint()const { return start_point_; }
	void setPushRange(double range);
	double pushRange()const { return push_range_; }
	void setPosition(double position);    
    void updatePosition();
	double getPosition() const;
	void setVolumeBounds(const Arry6d& bounds) { vol_bounds_ = bounds; }
	const auto& volumeBounds() { return vol_bounds_; }
	void setLookupTable(vtkLookupTable* table, std::size_t index);
	vtkLookupTable* getLookupTable(std::size_t index)const;
	void setPlaneType(Type t);
	Type planeType() const { return plane_type_; }
	void setSMPEnabled(bool enabled);
	bool isSMPEnabled()const { return smp_enabled_; }
	void setNormal(const Vec3& normal);
	Vec3 getNormal()const;
	void setOrigin(const Point3& origin);
	Point3 getOrigin()const;
	void setPoint1(const Point3& p1);
	Point3 getPoint1()const;
	void setPoint2(const Point3& p2);
	Point3 getPoint2()const;
	void setCenter(const Point3& center);
	Point3 getCenter()const;
    Point3 calcCenter()const;
	Point3 getTransformedCenter()const;
	Vec3 getTransformedNormal()const;
	Vec3 getTransformedViewUp1Vector()const;	
	Vec3 getViewUp1Vector()const;
	Vec3 getTransformedViewUp2Vector()const;	
	Vec3 getViewUp2Vector()const;
	double getSize1()const;
	double getSize2()const;
	void setVisibility(bool yes_no, std::size_t index, const Scene* scene);
	bool getVisibility(std::size_t index, const Scene* scene)const;
	void setOpacity(const Scene* scene, double alpha, std::size_t index);
	double getOpacity(const Scene* scene, std::size_t index)const;
	void setResliceInterpolate(bool yes_no);
	bool resliceInterpolate()const { return reslice_interpolate_; }
	void setTextureInterpolate(bool yes_no);
	bool textureInterpolate()const { return texture_interpolate_; }
	void addTextureOffScene(Scene* scene);
	bool isTextureOffScene(Scene* scene)const;
	void setRestrictToVolume(bool yes_no);
	bool restrictToVolume()const { return restrict_to_volume_; }
	SlicePlaneSprite();
	~SlicePlaneSprite();

private:
	void removeAllInputs();
	void updateNormal();
	void updateOrigin();
	void updateOutputFormatInfo(std::size_t clr_index, vtkLookupTable* lookup_table);
	void replaceLookupTable(vtkLookupTable* lookup_table, std::size_t index);
	void changeSliecPlaneTextureInterpolate(bool yes_no);
	void setPlaneActorVisibility(const Scene* target_scene, std::size_t input_idx, bool is_visible);
	bool getPlaneActorVisibility(const Scene& target_scene, std::size_t input_idx)const;
	void addPlaneActor(std::size_t input_idx);
	void removeSlicePlaneActor(std::size_t input_idx);
	void makeActors(Scene& scene) override;
	vtkActor* makePlaneActor(std::size_t i, bool no_texture = false);    
    struct InputProcessChain
    {
        vtkSmartPointer<vtkImageData> input;
        vtkSmartPointer<vtkImageReslice> reslicer;
        vtkSmartPointer<vtkTextureMapToPlane> texture_to_plane_mapper;
        vtkSmartPointer<vtkImageMapToColors> image_to_colors_mapper;
        bool visibility = true;
    };
    std::vector<InputProcessChain> process_chains_;
	std::vector<Scene*> texture_off_scenes_;
	vtkSmartPointer<vtkPlaneSource> plane_source_;
	vtkSmartPointer<vtkMatrix4x4> reslice_axes_;	
	vtkSmartPointer<vtkPlane> plane_equation_;
	Type plane_type_ = kCustom;
	bool reslice_interpolate_ = true;
	bool texture_interpolate_ = true;
	bool smp_enabled_ = true;
	bool restrict_to_volume_ = false;
	Arry6d vol_bounds_ = {0.0};
	Point3 start_point_ = {0.0, 0.0, 0.0};
	double push_range_ = 0.0;
};

NAMESPACE_END

#endif