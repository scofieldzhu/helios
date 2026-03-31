/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/5/8
*******************************************************/
#ifndef __camera_interactor_style_h__
#define __camera_interactor_style_h__

#include <vtkCommand.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include "mirfak/core/mirfak_core_export.h"
#include "mirfak/core/mirfak_core_typedef.h"

MIRFAK_NAMESPACE_BEGIN

class MIRFAK_CORE_API CameraInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
	vtkTypeMacro(CameraInteractorStyle, vtkInteractorStyleTrackballCamera);
    enum _Event{
        MouseFreeMoving = vtkCommand::UserEvent + 1
    };
    struct FreeMoveData{
        int x = 0; 
        int y = 0;
        std::optional<double> scalar;
        PlaneScene* event_scene = 0;
    };
	static CameraInteractorStyle* New();
    void enableFreeMovePick(bool enabled);
	void OnLeftButtonDown() override;
	void OnLeftButtonUp() override;
	void OnRightButtonDown() override;
	void OnRightButtonUp() override;
    void OnMiddleButtonDown() override;
    void OnMiddleButtonUp() override;
    void Pan() override;
    void Dolly(double factor) override;
    void OnMouseMove() override;
	void OnChar() override;
    void OnLeave() override;
    void cacheCurrentScale();
    void setMinMaxScale(double minScale, double maxScale);
    void setInteractorEnable(bool enable);
	bool getInteractorEnable()const { return interactor_enabled_; }
    void OnMouseWheelForward() override;
    void OnMouseWheelBackward() override;

private:
    void Dolly2D(double factor);
    std::optional<double> pickImageScalar(int x, int y, PlaneScene* s)const;
	CameraInteractorStyle();
	~CameraInteractorStyle();
    bool left_button_down_ = false;
    bool right_button_down_ = false;
    bool middle_button_down_ = false;
    double min_scale_factor_ = 0.3;
    double max_scale_factor_ = 30;
    double current_scale_factor_ = -1;
    bool interactor_enabled_ = true;
    vtkRenderer* target_event_renderer_ = nullptr;
    FreeMoveData current_free_moving_data_;
    vtkRenderer* last_move_renderer_ = nullptr;
    bool free_move_pick_enabled_ = false;
};

NAMESPACE_END

#endif 
