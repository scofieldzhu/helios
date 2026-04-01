/******************************************************** 
* author: scofieldzhu
* time:2025/7/24
*******************************************************/
#include "polygon_editor.h"
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkAssemblyPath.h>
#include <vtkHardwareSelector.h>
#include <vtkDataObject.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkVolumePicker.h>
#include "polygon_sprite.h"
#include "helios/core/scene.h"
#include "helios/basic/log_service.h"
#include "helios/core/render_widget.h"
#include "helios/core/sprite_cell_picker.h"

HELIOS_NAMESPACE_BEGIN

vtkStandardNewMacro(PolygonEditor);

PolygonEditor::PolygonEditor()
{
    action_slot_mapper_.setActionSlot(
        vtkCommand::LeftButtonPressEvent,
        ActionEvent::Select,
        this,
        PolygonEditor::SelectAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::LeftButtonReleaseEvent,
        ActionEvent::EndSelect,
        this, 
        PolygonEditor::EndSelectAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::MiddleButtonPressEvent,
        ActionEvent::AddPoint,
        this, 
        PolygonEditor::AddPointAction
    );
    action_slot_mapper_.setActionSlot(
        vtkCommand::MouseMoveEvent,
        ActionEvent::Translate,
        this,
        PolygonEditor::TranslateAction
    );
}

PolygonEditor::~PolygonEditor()
{
}

void PolygonEditor::setCurrentVolume(vtkProp* v)
{
    current_volume_ = v;
}

bool PolygonEditor::setCurrentSprite(Sprite *s)
{
    if(s == current_sprite_){
        return true;
    }
    if(s && PolygonSprite::SafeDownCast(s)){
        current_sprite_ = s;
        current_polygon_ = PolygonSprite::SafeDownCast(s);
        return true;
    }
    return false;
}

void PolygonEditor::SelectAction(Editor* e)
{
    auto self = reinterpret_cast<PolygonEditor*>(e);
    self->handleSelectAction();
}

void PolygonEditor::EndSelectAction(Editor* e)
{
    auto self = reinterpret_cast<PolygonEditor*>(e);
    self->handleUnselectAction();
}

void PolygonEditor::TranslateAction(Editor *e)
{
    auto self = reinterpret_cast<PolygonEditor*>(e);
    self->handleTranslateAction();
}

void PolygonEditor::EndTranslateAction(Editor *e)
{
    auto self = reinterpret_cast<PolygonEditor*>(e);
    self->handleEndTranslateAction();
}

void PolygonEditor::AddPointAction(Editor *e)
{
    auto self = reinterpret_cast<PolygonEditor*>(e);
    self->handleAddPointAction();
}

void PolygonEditor::handleSelectAction()
{
    SPDLOG_TRACE("handleSelectAction");
    int x, y;
    current_interactor_->GetEventPosition(x, y);    
    if(editor_state_ == ES_START || editor_state_ == ES_DEFINE){
        if(current_polygon_ == nullptr){
            //abortCurrentEventTransmission(false);
            return;
        }
        auto picked_ctrlpt_id = pickControlPointId(x, y);
        current_polygon_->setControlPointSelected(picked_ctrlpt_id);
        current_polygon_->render();
        if(picked_ctrlpt_id != -1){
            editor_state_ = ES_MANIPULATE;
        }
        SPDLOG_TRACE("Picked point id:{}!", picked_ctrlpt_id);
    }    
}

void PolygonEditor::handleUnselectAction()
{
    SPDLOG_TRACE("handleUnselectAction");
    
    if(editor_state_ == ES_MANIPULATE){
        current_polygon_->setControlPointSelected(-1);
        current_polygon_->render();
    }
    editor_state_ = ES_START;
    
}

void PolygonEditor::handleTranslateAction()
{
    SPDLOG_TRACE("handleTranslateAction");
    if(editor_state_ == ES_MANIPULATE){
        assert(current_polygon_->selectedControlPointId() != -1);
        if(current_polygon_ == nullptr){
            return;
        }
        int x, y;
        current_interactor_->GetEventPosition(x, y);    
        auto pt = pickWorldPoint(x, y);
        if(!pt){
            SPDLOG_WARN("Pick world point failed!");
            return;
        }
        if(current_polygon_->selectedControlPointId() != -1){
            current_polygon_->setPoint(current_polygon_->selectedControlPointId(), pt.value());
            current_polygon_->render();
        }
    }

}

void PolygonEditor::handleEndTranslateAction()
{
    SPDLOG_TRACE("handleEndTranslateAction");
}

void PolygonEditor::handleAddPointAction()
{
    SPDLOG_TRACE("handleAddPointAction");
    int x, y;
    current_interactor_->GetEventPosition(x, y);    
    if(editor_state_ == ES_START || editor_state_ == ES_DEFINE){
        editor_state_ = ES_DEFINE;
        if(current_polygon_ == nullptr){
            //abortCurrentEventTransmission(false);
            return;
        }
        auto future_pt = pickWorldPoint(x, y);
        if(!future_pt){ //no world point picked!
            SPDLOG_TRACE("pickWorldPoint failed!");
            return;
        }
        current_polygon_->appendPoint(future_pt.value());
        current_polygon_->render();
    } 
}

Pt3Opt PolygonEditor::pickWorldPoint(int x, int y)
{
    Pt3Opt pt;
    if(current_volume_ && current_volume_->GetVisibility()){
        vtkNew<vtkVolumePicker> vol_picker;
        vol_picker->PickFromListOn();
        vol_picker->AddPickList(current_volume_);
        const bool ok = vol_picker->Pick(x, y, 0.0, current_scene_->getRenderer());
        if(ok){
            Point3 out_point;
            vol_picker->GetPickPosition(out_point);
            pt = out_point;
            SPDLOG_DEBUG("vol picked!");
        }
    }
    if(!pt){
        SpriteCellPicker scp;
        scp.setExcludedSprites({current_polygon_});
        auto pinfos = scp.pick(*current_scene_, x, y);
        if(!pinfos.empty()){
            pt = pinfos[0].position;
            SPDLOG_DEBUG("SpriteCellPicker picked! pt:{} sprite:{}", pinfos[0].position.toStr(), pinfos[0].sprite->getClsTypeName());
        }
        // pt = current_scene_->grabWorldPointFromPixel(x, y);
        // if(pt){
        //     SPDLOG_DEBUG("grab picked! pt:{}", pt.value().toStr());
        // }
    }
    return pt;
}

int PolygonEditor::pickControlPointId(int x, int y)
{
    vtkNew<vtkHardwareSelector> sel;
    sel->SetRenderer(current_scene_->getRenderer());
    sel->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_VERTICES);
    sel->SetArea(x, y, x, y);        // 单点选
    vtkSmartPointer<vtkSelection> res = sel->Select();
    auto selected_ids = current_polygon_->getControlPointIdsFromSelection(sel, res);
    if(!selected_ids.empty()){
        return selected_ids.front();
    }
    return -1;
}

NAMESPACE_END


