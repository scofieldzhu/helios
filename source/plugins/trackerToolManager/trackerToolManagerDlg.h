/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/25
*******************************************************/
#ifndef __trackerToolManagerDlg_h__
#define __trackerToolManagerDlg_h__

#include "ui_trackerToolManagerDlg.h"
#include "mirfak/core/render_widget.h"
#include "mirfak/navigation/handpiece_sprite.h"

namespace mirfak{
	class OrthoPlanesSprite;
}

class QTimer;

class TrackerToolManagerDlg : public QWidget
{
	Q_OBJECT
public:
	void setRenderWidget(mirfak::RenderWidget *sw);
	TrackerToolManagerDlg(QWidget* p);
	~TrackerToolManagerDlg();

private slots:
    void slotShowHpButtonToggled(bool chk);
    void slotCurrentDrillSwitched(const QString &name);
    void slotDrillRotatingTimeout();
    void slotRotateButtonToggled(bool chk);
	void slotConBtnPressed();
	void slotDisConBtnPressed();
	void slotTrackingStarted();
	void slotTrackingStopped();
	void slotTrackerTimerPolling();
    
private:
	Ui::Form ui_;
    mirfak::RenderWidget* render_widget_ = nullptr;
    mirfak::Scene* axial_scene_ = nullptr;
	mirfak::Scene* coronal_scene_ = nullptr;
	mirfak::Scene* sagittal_scene_ = nullptr;
	mirfak::Scene* ortho_scene_ = nullptr;
    std::unique_ptr<mirfak::HandpieceSprite> handpiece_;
    QTimer* drill_rotating_timer_;
};

#endif