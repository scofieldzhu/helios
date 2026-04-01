/*******************************************************
* author: scofieldzhu
* time:2025/10/30
*******************************************************/
#ifndef __point_move_tool_sprite_h__
#define __point_move_tool_sprite_h__

#include "helios/sprites/arrow_sprite.h"
#include "helios/sprites/torus_sprite.h"

class vtkCellPicker;

HELIOS_NAMESPACE_BEGIN

class HELIOS_SPRITES_API PointMoveToolSprite : public Sprite
{
	SPRITE_DECL(PointMoveToolSprite, Sprite)
public:
	Signal<Point3> CenterChanged;
	void setCenter(const Point3& pt);
	const Point3& center()const;
	void setAxisXDirection(const Vec3& n);
	Vec3 getAxisXDirection()const;
	void setAxisYDirection(const Vec3& n);
	Vec3 getAxisYDirection()const;
	void setAxisZDirection(const Vec3& n);
	Vec3 getAxisZDirection()const;
	enum EditState{
		ES_NONE,
		ES_OUTSIDE,
		ES_AXIS_X_ROTATE,
		ES_AXIS_X_TRANSLATE,
		ES_AXIS_Y_ROTATE,
		ES_AXIS_Y_TRANSLATE,
		ES_AXIS_Z_ROTATE,
		ES_AXIS_Z_TRANSLATE
	};
	struct EditStateData{
		int state = 0;
		Pt3Opt world_point;
		Vec3Opt translate_axis_normal;
		Vec3Opt rotate_axis_normal;
	};
	int computeEditState(Scene& s, int x, int y, int modify = 0) override;
	const void* getEditStateData()const override{ return &edit_state_data_; }
	PointMoveToolSprite();
	~PointMoveToolSprite();

private:	
	void createSource(Scene& scene) override;
	void makeActors(Scene& scene) override;
	std::unique_ptr<ArrowSprite> axis_x_, axis_y_, axis_z_;
	std::unique_ptr<TorusSprite> torus_x_, torus_y_, torus_z_;
	using PickerPtr = vtkSmartPointer<vtkCellPicker>;
	std::map<const Scene*, PickerPtr> picker_dict_;
	EditStateData edit_state_data_;
};

NAMESPACE_END

#endif