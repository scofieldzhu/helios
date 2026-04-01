/******************************************************** 
* author: scofieldzhu
* time:2025/11/25
*******************************************************/
#ifndef __trackerToolManagerDlg_h__
#define __trackerToolManagerDlg_h__

#include "ui_trackerToolManagerDlg.h"
#include "helios/core/render_widget.h"
#include "helios/navigation/handpiece_sprite.h"

namespace helios{
	class OrthoPlanesSprite;
}

class QTimer;

class TrackerToolManagerDlg : public QWidget
{
	Q_OBJECT
public:
	void setRenderWidget(helios::RenderWidget *sw);
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
    helios::RenderWidget* render_widget_ = nullptr;
    helios::Scene* axial_scene_ = nullptr;
	helios::Scene* coronal_scene_ = nullptr;
	helios::Scene* sagittal_scene_ = nullptr;
	helios::Scene* ortho_scene_ = nullptr;
    std::unique_ptr<helios::HandpieceSprite> handpiece_;
    QTimer* drill_rotating_timer_;
};

#endif