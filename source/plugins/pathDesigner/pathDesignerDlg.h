/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/24
*******************************************************/
#ifndef __pathDesignerDlg_h__
#define __pathDesignerDlg_h__

#include "ui_pathDesignerDlg.h"
#include "mirfak/sprites/surgical_path_editor.h"
#include "mirfak/sprites/point_move_tool_editor.h"

namespace mirfak{
	class OrthoPlanesSprite;
}

class PathDesigner;

class PathDesignerDlg : public QWidget
{
	Q_OBJECT
public:
	void setRenderWidget(mirfak::RenderWidget *sw);
	PathDesignerDlg(QWidget* p, PathDesigner* pd);
	~PathDesignerDlg();

signals:
	void pathCreated(QString name);

private slots:
	void slotCreateSurgicalPathButtonToggled(bool chk);
	void slotPathEditToggled(bool chk);
	void slotPathEdit2Toggled(bool chk);
	void slotPathCreated(QString path_name);
	void slotDeletePathButtonPressed();
	void slotChangePathClrButtonPressed();
	void slotCurrentPathChanged(const QString& item_text);

private:
	void onPathCreated(std::string path_name);
	Ui::Form ui_;
	std::shared_ptr<mirfak::SurgicalPathSprite> current_surgical_path_;
	vtkSmartPointer<mirfak::SurgicalPathEditor> surgical_path_editor_;
	unsigned int path_create_event_bind_id_ = -1;
	mirfak::RenderWidget* render_widget_ = nullptr;
	mirfak::Scene* axial_scene_ = nullptr;
	mirfak::Scene* coronal_scene_ = nullptr;
	mirfak::Scene* sagittal_scene_ = nullptr;
	mirfak::Scene* ortho_scene_ = nullptr;
	std::unique_ptr<mirfak::PointMoveToolSprite> point_move_tool_;
	vtkSmartPointer<mirfak::PointMoveToolEditor> point_move_tool_editor_;
	mirfak::OrthoPlanesSprite* orthoplanes_ = nullptr;
	PathDesigner* pd_;
};

#endif