/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2026)
* author: zhucg
* time:2025/11/24
*******************************************************/
#include "pathDesignerDlg.h"
#include <QColorDialog>
#include <vtkPlane.h>
#include "helios/sprites/surgical_path_editor.h"
#include "helios/core/render_widget.h"
#include "helios/sprites/abstract_volume_sprite.h"
#include "helios/sprites/polydata_cutter_sprite.h"
#include "helios/sprites/ortho_planes_sprite.h"
#include "helios/core/slice_plane_sprite.h"
#include "helios/core/scene_repository.h"
#include "pathDesigner.h"

using namespace helios;

PathDesignerDlg::PathDesignerDlg(QWidget* p, PathDesigner* pd)
	:QWidget(p),
    pd_(pd)
{
	ui_.setupUi(this);
    connect(ui_.create_path_cb, &QCheckBox::toggled, this, &PathDesignerDlg::slotCreateSurgicalPathButtonToggled);
    connect(ui_.edit_path_cb, &QCheckBox::toggled, this, &PathDesignerDlg::slotPathEditToggled);
    connect(ui_.edit_path2_cb, &QCheckBox::toggled, this, &PathDesignerDlg::slotPathEdit2Toggled);
    connect(this, &PathDesignerDlg::pathCreated, this, &PathDesignerDlg::slotPathCreated);
    connect(ui_.del_path_btn, &QAbstractButton::pressed, this, &PathDesignerDlg::slotDeletePathButtonPressed);
    connect(ui_.change_path_clr_btn, &QAbstractButton::pressed, this, &PathDesignerDlg::slotChangePathClrButtonPressed);
    connect(ui_.paths_cb, &QComboBox::currentTextChanged, this, &PathDesignerDlg::slotCurrentPathChanged);        
    ui_.point_type_cb->addItem(tr("Start Point"));
    ui_.point_type_cb->addItem(tr("End Point"));
}

PathDesignerDlg::~PathDesignerDlg ()
{

}

void PathDesignerDlg::slotCreateSurgicalPathButtonToggled(bool chk)
{
    if(chk){
        //if(surgical_path_editor_){
        //    surgical_path_editor_->PathCreatedSignal.unbind(path_create_event_bind_id_);
        //}
        ui_.edit_path_cb->setChecked(false);
        if(surgical_path_editor_ == nullptr){
            surgical_path_editor_ = SurgicalPathEditor::New();
            surgical_path_editor_->setEditorAction(SurgicalPathEditor::EA_CREATE_PATH);
            path_create_event_bind_id_ = surgical_path_editor_->PathCreatedSignal.bind(std::bind(&PathDesignerDlg::onPathCreated, this, std::placeholders::_1));
            surgical_path_editor_->setSurgicalPathGroup(pd_->getPathGroup());
            surgical_path_editor_->setAllowedScenes({ortho_scene_, axial_scene_, coronal_scene_, sagittal_scene_});
            surgical_path_editor_->setCurrentInteractor(render_widget_->interactor());   
            auto existing_volume = AbstractVolumeSprite::SafeDownCast(ortho_scene_->getSprite("Volume"));  
            surgical_path_editor_->setCurrentVolume(existing_volume);        
        }else{
            surgical_path_editor_->setEditorAction(SurgicalPathEditor::EA_CREATE_PATH);
        }
        surgical_path_editor_->setEnabled(true);
    }else{
        if(surgical_path_editor_){
            surgical_path_editor_->setEnabled(false);
        }        
    }
}

void PathDesignerDlg::onPathCreated(std::string path_name)
{
    auto created_path = pd_->getPathGroup()->find(path_name);
    Q_ASSERT(created_path);
    auto gp = created_path->createComponentGroup("Cutter");
    auto new_cutter = std::make_shared<PolyDataCutterSprite>();
    new_cutter->setPolyDataTransform(created_path->getTransform());
    new_cutter->setInputPolyData(created_path->getPolyData());
    new_cutter->setCutPlaneEquation(orthoplanes_->getAxialPlane()->getPlaneEquation());
    new_cutter->setColor(Color::Red);
    new_cutter->connectScene(*axial_scene_);
    gp->append(new_cutter);

    new_cutter = std::make_shared<PolyDataCutterSprite>();
    new_cutter->setPolyDataTransform(created_path->getTransform());
    new_cutter->setInputPolyData(created_path->getPolyData());
    new_cutter->setCutPlaneEquation(orthoplanes_->getCoronalPlane()->getPlaneEquation());
    new_cutter->setColor(Color::Red);
    new_cutter->connectScene(*coronal_scene_);
    gp->append(new_cutter);   

    new_cutter = std::make_shared<PolyDataCutterSprite>();
    new_cutter->setPolyDataTransform(created_path->getTransform());
    new_cutter->setInputPolyData(created_path->getPolyData());
    new_cutter->setCutPlaneEquation(orthoplanes_->getSagittalPlane()->getPlaneEquation());
    new_cutter->setColor(Color::Red);
    new_cutter->connectScene(*sagittal_scene_);
    gp->append(new_cutter);

    emit pathCreated(QString::fromStdString(path_name));
}

void PathDesignerDlg::slotPathEditToggled(bool chk)
{
    if(ui_.paths_cb->currentIndex() < 0){
        return;
    }
    auto path_sprite = pd_->getPathGroup()->find(ui_.paths_cb->currentText().toStdString());
    if(path_sprite == nullptr){
        return;
    }
    if(chk){
        ui_.create_path_cb->setChecked(false);
        if(surgical_path_editor_ == nullptr){
            surgical_path_editor_ = SurgicalPathEditor::New();            
            surgical_path_editor_->setAllowedScenes({ortho_scene_, axial_scene_, coronal_scene_, sagittal_scene_});
            surgical_path_editor_->setCurrentInteractor(render_widget_->interactor());   
            auto existing_volume = AbstractVolumeSprite::SafeDownCast(ortho_scene_->getSprite("Volume"));  
            surgical_path_editor_->setCurrentVolume(existing_volume);                    
        }
        surgical_path_editor_->setEditorAction(SurgicalPathEditor::EA_EDIT_PATH);
        surgical_path_editor_->setCurrentSprite(path_sprite);
        surgical_path_editor_->setEnabled(true);
        
    }else{
        if(surgical_path_editor_){
            surgical_path_editor_->setEnabled(false);
        }
    }
    path_sprite->render();
}

void PathDesignerDlg::slotPathEdit2Toggled(bool chk)
{
    if(chk){
        if(ui_.paths_cb->currentIndex() < 0){
            return;
        }
        auto current_path = pd_->getPathGroup()->find(ui_.paths_cb->currentText().toStdString());
        if(current_path == nullptr){
            return;
        }
        ui_.create_path_cb->setChecked(false);
        ui_.edit_path_cb->setChecked(false);
        surgical_path_editor_ = nullptr;

        int point_type = ui_.point_type_cb->currentIndex();
        point_move_tool_ = std::make_unique<PointMoveToolSprite>();
        point_move_tool_->setColor(Color::Red);            
        point_move_tool_->setCenter(point_type == 0 ? current_path->startPoint() : current_path->endPoint());
        point_move_tool_->CenterChanged.bind([point_type, current_path](Point3 pt){
            if(point_type == 0){    
                current_path->setStartPoint(pt);
            }else{
                current_path->setEndPoint(pt);
            }
            current_path->render();
        });
        Vec3 v = current_path->startPoint() - current_path->endPoint();
        v.normalize();
        Line proj_line(current_path->endPoint(), v);
        auto proj = proj_line.projectPoint({0.0, 0.0, 0.0});
        Vec3 dir_x = Point3(0.0, 0.0, 0.0) - proj;
        dir_x.normalize();
        Vec3 dir_y, dir_z;
        if(point_type == 0){   
            dir_z = v;
        }else{
            dir_z = current_path->endPoint() - current_path->startPoint();
            dir_z.normalize();
        }
        dir_y = dir_x.crossed(dir_z);
        dir_y.normalize();
        point_move_tool_->setAxisXDirection(dir_x);
        point_move_tool_->setAxisYDirection(dir_y);
        point_move_tool_->setAxisZDirection(dir_z);
        point_move_tool_->connectScene(*ortho_scene_);
        point_move_tool_editor_ = PointMoveToolEditor::New();
        point_move_tool_editor_->setCurrentSprite(point_move_tool_.get());
        point_move_tool_editor_->setAllowedScenes({ortho_scene_});
        point_move_tool_editor_->setCurrentInteractor(render_widget_->interactor());   
        point_move_tool_editor_->setEnabled(true);
    }else{
        if(point_move_tool_editor_){
            point_move_tool_editor_->setEnabled(false);
        }
        point_move_tool_ = nullptr;
    }
    ortho_scene_->render();
}

void PathDesignerDlg::slotPathCreated(QString path_name)
{
    ui_.paths_cb->addItem(path_name);
    ui_.paths_cb->setCurrentText(path_name);
}

void PathDesignerDlg::slotDeletePathButtonPressed()
{
    QString path_name = ui_.paths_cb->currentText();
    ui_.paths_cb->removeItem(ui_.paths_cb->currentIndex());
    pd_->getPathGroup()->remove(path_name.toStdString());
    render_widget_->render();
}

void PathDesignerDlg::slotChangePathClrButtonPressed()
{
    QString path_name = ui_.paths_cb->currentText();
    auto path_sprite = pd_->getPathGroup()->find(path_name.toStdString());
    Q_ASSERT(path_sprite);
    QColor color = QColorDialog::getColor(Qt::red, this, "选择颜色");
    if(color.isValid()){
        Color new_clr(color.red(), color.green(), color.blue());
        path_sprite->setColor(new_clr);
        path_sprite->render();
        QString clr_hex_str = QString::fromStdString(new_clr.toHexString());
        QString new_style = QString("background-color:%1;").arg(clr_hex_str);
        ui_.change_path_clr_btn->setStyleSheet(new_style);
        ui_.change_path_clr_btn->style()->unpolish(ui_.change_path_clr_btn);
        ui_.change_path_clr_btn->style()->polish(ui_.change_path_clr_btn);
        ui_.change_path_clr_btn->update();
    }
}

void PathDesignerDlg::slotCurrentPathChanged(const QString& item_text)
{
    auto cur_path = pd_->getPathGroup()->beginTraverse();
    auto selected_path = cur_path;
    while(cur_path){
        if(cur_path->name() == item_text.toStdString()){
            cur_path->highlight(Color::Yellow);
            selected_path = cur_path;
        }else{
            cur_path->unhighlight();
        }
        cur_path = pd_->getPathGroup()->next();
    }
    ortho_scene_->render();
    QString clr_hex_str = QString::fromStdString(selected_path->getColor().value().toHexString());
    QString new_style = QString("background-color:%1;").arg(clr_hex_str);
    ui_.change_path_clr_btn->setStyleSheet(new_style);
    ui_.change_path_clr_btn->style()->unpolish(ui_.change_path_clr_btn);
    ui_.change_path_clr_btn->style()->polish(ui_.change_path_clr_btn);
    ui_.change_path_clr_btn->update();

}

void PathDesignerDlg::setRenderWidget(RenderWidget* sw)
{
    render_widget_ = sw;
    if(render_widget_ == nullptr){
        return;
    }
    axial_scene_ = render_widget_->sceneRepository().getScene(Axial);
    coronal_scene_ = render_widget_->sceneRepository().getScene(Coronal);
    sagittal_scene_ = render_widget_->sceneRepository().getScene(Sagittal);
    ortho_scene_ = render_widget_->sceneRepository().getScene(Ortho);
    orthoplanes_ = OrthoPlanesSprite::SafeDownCast(ortho_scene_->getSprite("OrthoPlanes"));
}
