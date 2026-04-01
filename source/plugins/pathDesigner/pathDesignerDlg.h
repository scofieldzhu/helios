/******************************************************** 
* author: scofieldzhu
* time:2025/11/24
*******************************************************/
#ifndef __pathDesignerDlg_h__
#define __pathDesignerDlg_h__

#include "ui_pathDesignerDlg.h"
#include "helios/sprites/surgical_path_editor.h"
#include "helios/sprites/point_move_tool_editor.h"

namespace helios{
	class OrthoPlanesSprite;
}

class PathDesigner;

class PathDesignerDlg : public QWidget
{
	Q_OBJECT
public:
	void setRenderWidget(helios::RenderWidget *sw);
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
	std::shared_ptr<helios::SurgicalPathSprite> current_surgical_path_;
	vtkSmartPointer<helios::SurgicalPathEditor> surgical_path_editor_;
	unsigned int path_create_event_bind_id_ = -1;
	helios::RenderWidget* render_widget_ = nullptr;
	helios::Scene* axial_scene_ = nullptr;
	helios::Scene* coronal_scene_ = nullptr;
	helios::Scene* sagittal_scene_ = nullptr;
	helios::Scene* ortho_scene_ = nullptr;
	std::unique_ptr<helios::PointMoveToolSprite> point_move_tool_;
	vtkSmartPointer<helios::PointMoveToolEditor> point_move_tool_editor_;
	helios::OrthoPlanesSprite* orthoplanes_ = nullptr;
	PathDesigner* pd_;
};

#endif