/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/6/19
*******************************************************/
#include "polygon_sprite.h"
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkLineSource.h>
#include <vtkGlyph3DMapper.h>
#include <vtkGlyphSource2D.h>
#include <vtkPointData.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkPropPicker.h>
#include <vtkCellPicker.h>
#include <vtkPoints.h>
#include <vtkPointLocator.h>
#include <vtkUnsignedCharArray.h>
#include <vtkIdTypeArray.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkHardwareSelector.h>
#include <vtkInformation.h>
#include <vtkRenderer.h>
#include "mirfak/basic/coord_conv.h"
#include "mirfak/basic/log_service.h"
#include "mirfak/core/scene.h"
#include "mirfak/core/render_widget.h"

MIRFAK_NAMESPACE_BEGIN

PolygonSprite::PolygonSprite()
    :points_(vtkSmartPointer<vtkPoints>::New()),
    lines_(vtkSmartPointer<vtkCellArray>::New()),
    polys_(vtkSmartPointer<vtkPolyData>::New()),
    close_line_source_(vtkSmartPointer<vtkLineSource>::New()),
    gly_polys_(vtkSmartPointer<vtkPolyData>::New()),
    cam_dir_array_(vtkSmartPointer<vtkFloatArray>::New()),
    gly_colors_(vtkSmartPointer<vtkUnsignedCharArray>::New()),
    line_property_(vtkSmartPointer<vtkProperty>::New()),
    id_array_(vtkSmartPointer<vtkIdTypeArray>::New())
{
    close_line_source_->SetResolution(100);
    gly_polys_->SetPoints(points_);

    cam_dir_array_->SetName("CamDir");
    cam_dir_array_->SetNumberOfComponents(3);
    cam_dir_array_->InsertNextTuple3(0, 0, -1); //fill a arbitrary data!!
    gly_polys_->GetPointData()->AddArray(cam_dir_array_);

    gly_colors_->SetName("GlyphColor");
    gly_colors_->SetNumberOfComponents(3);
    gly_polys_->GetPointData()->AddArray(gly_colors_);

    id_array_->SetName("SelectionId");
    id_array_->SetNumberOfComponents(1);
    gly_polys_->GetPointData()->AddArray(id_array_);

    polys_->SetPoints(points_);
    polys_->SetLines(lines_);
}

PolygonSprite::~PolygonSprite()
{
}

void PolygonSprite::setClosed(bool flag)
{
    closed_ = flag;
}

void PolygonSprite::setPoints(const Pt3List& pts)
{
    points_->Reset();
    for(const auto& pt : pts){
        points_->InsertNextPoint(pt);
    }    
    points_->Modified();
    cam_dir_array_->SetNumberOfTuples(points_->GetNumberOfPoints());
    updatePolydata();
    updateCloseLineData();        
}

void PolygonSprite::appendPoint(const Point3& pt)
{
    points_->InsertNextPoint(pt);
    points_->Modified();
    cam_dir_array_->SetNumberOfTuples(points_->GetNumberOfPoints());
    updatePolydata();
    updateCloseLineData();
}

void PolygonSprite::insertPoint(const Point3 &pt, int pos)
{
    points_->InsertPoint(pos, pt);
    points_->Modified();
    cam_dir_array_->SetNumberOfTuples(points_->GetNumberOfPoints());
    updatePolydata();
    updateCloseLineData();
}

void PolygonSprite::registerPickers()
{
    if(!existsDisplayScene()){
        return;
    }
    auto rw = getDisplaySceneList().front()->sceneWidget()->renderWindow();
    //registerPicker(rw, picker_);
}

void PolygonSprite::unRegisterPickers()
{
}

void PolygonSprite::setPoint(int id, Point3 &pt)
{
    if(id >= 0 && id < points_->GetNumberOfPoints()){
        points_->SetPoint(id, pt);
        points_->Modified();
        updatePolydata();
        updateCloseLineData();
    }
}

Pt3Opt PolygonSprite::getPoint(int id) const
{
    if(id >= 0 && id < points_->GetNumberOfPoints()){
        Point3 pt = points_->GetPoint(id);
        return pt;
    }
    return std::nullopt;
}

Pt3List PolygonSprite::getPoints() const
{
    return FromVtkPoints(points_);
}

void PolygonSprite::updatePolydata()
{
    lines_->Reset();
    gly_colors_->SetNumberOfTuples(points_->GetNumberOfPoints());
    id_array_->SetNumberOfTuples(points_->GetNumberOfPoints());
    for(auto i = 0; i < points_->GetNumberOfPoints(); ++i){
        Color clr = (selected_control_point_id_ == i ? ctrl_point_select_clr_ : ctrl_point_clr_);
        gly_colors_->SetTuple3(i, clr.red(), clr.green(), clr.blue());
        id_array_->SetValue(i, i);
        if(i > 0){
            lines_->InsertNextCell({i - 1, i});
        }
    }
    lines_->Modified();
    polys_->Modified();
    id_array_->Modified();
    gly_colors_->Modified();
    gly_polys_->Modified();
    modified();
}

void PolygonSprite::updateCloseLineData()
{
    if(points_->GetNumberOfPoints() >= 2 && closed_){
        Point3 end_pt  = points_->GetPoint(points_->GetNumberOfPoints() - 1);
        Point3 last_pt = points_->GetPoint(0);
        close_line_source_->SetPoint1(last_pt);
        close_line_source_->SetPoint2(end_pt);
    }
}

void PolygonSprite::createSource(Scene& scene)
{
    updatePolydata();
    updateCloseLineData();
}

void PolygonSprite::makeActors(Scene& scene)
{
    makePolygonActor(scene);
    if(closed_){
        makeEndLineActor(scene);
    }
}

void PolygonSprite::makePolygonActor(Scene& scene)
{
    {
        vtkNew<vtkGlyphSource2D> gly_source_2d;
        gly_source_2d->SetGlyphTypeToCircle();
        gly_source_2d->SetResolution(50);
        gly_source_2d->FilledOn();
        gly_source_2d->SetScale(3.0);
        vtkNew<vtkTransform> rotation;
        rotation->RotateY(-90);                       // 关键：Z 轴转到 X 轴
        vtkNew<vtkTransformPolyDataFilter> polys_transformer;
        polys_transformer->SetTransform(rotation);
        polys_transformer->SetInputConnection(gly_source_2d->GetOutputPort());
        vtkNew<vtkGlyph3DMapper> mapper;
        mapper->SetInputData(gly_polys_);
        mapper->SetSourceConnection(polys_transformer->GetOutputPort());
        mapper->SetOrientationArray("CamDir");
        mapper->SetOrientationModeToDirection();        
        mapper->OrientOn();
        mapper->ScalarVisibilityOn();
        mapper->SelectColorArray("GlyphColor");
        mapper->SetScalarModeToUsePointFieldData();
        mapper->SetColorModeToDirectScalars();

        mapper->SetUseSelectionIds(true);
        mapper->SetSelectionIdArray("SelectionId");

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        addSceneProp(scene, actor, "gly_Polygon");
        // picker_->InitializePickList();
        // picker_->AddPickList(actor);
        //actor->GetProperty()->SetColor(1, 1, 0);  
        //actor->GetProperty()->SetOpacity(0.75);
        vtkNew<vtkCallbackCommand> cb;
        cb->SetClientData(cam_dir_array_);
        cb->SetCallback([](vtkObject* cam_obj, unsigned long, void* dir_array, void*){
                auto camera  = static_cast<vtkCamera*>(cam_obj);
                auto cam_dir_array  = static_cast<vtkFloatArray*>(dir_array);
                Point3 new_dir = camera->GetDirectionOfProjection();
                for(auto i = 0; i< cam_dir_array->GetNumberOfTuples(); ++i){
                    cam_dir_array->SetTuple(i, new_dir);
                } 
                cam_dir_array->Modified();                       
            }
        );
        scene.getRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent,cb);
    }
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(polys_);        
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        line_property_->SetColor(1.0, 0.0, 0.0);
        actor->SetProperty(line_property_);
        addSceneProp(scene, actor, "Polygon");
    }
}

void PolygonSprite::makeEndLineActor(Scene &scene)
{
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(close_line_source_->GetOutputPort());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    addSceneProp(scene, actor, "CloseLine");
    actor->GetProperty()->SetLineStipplePattern(0xf0f0);
    actor->GetProperty()->SetColor(1.0, 1.0, 0.0);
}

bool PolygonSprite::addToScene(Scene& scene)
{
    if(existsDisplayScene()){
        SPDLOG_WARN("Exists other scene is connected to this instance already!");
        return false;
    }
    return Sprite::addToScene(scene);
}

std::vector<int> PolygonSprite::getControlPointIdsFromSelection(vtkHardwareSelector* selector, vtkSelection* selection)
{    
    if(!existsDisplayScene()){
        return {};
    }
    if(selector == nullptr || selection == nullptr || selection->GetNumberOfNodes() == 0){
        return {};
    }
    auto scene = RenderWidget::RendererToScene(selector->GetRenderer());
    if(scene == nullptr){
        return {};
    }
    std::vector<int> result_ids;
    auto polygon_actor = getSceneActor("gly_Polygon", scene);
    for(auto ni = 0; ni < selection->GetNumberOfNodes(); ++ni){
        vtkSelectionNode* node = selection->GetNode(ni);
        if(!node->GetProperties()->Has(vtkSelectionNode::PROP_ID())){
            continue;
        }
        vtkIdType pid = node->GetProperties()->Get(vtkSelectionNode::PROP_ID());
        vtkProp* prop  = selector->GetPropFromID(static_cast<unsigned int>(pid));
        auto actor = vtkActor::SafeDownCast(prop);
        if(actor == nullptr || actor != polygon_actor){
            continue;
        }
        auto id_list = vtkIdTypeArray::SafeDownCast(node->GetSelectionList());
        if(id_list){
            for(auto i = 0; i < id_list->GetNumberOfTuples(); ++i){
                result_ids.push_back(id_list->GetValue(i));
            }
        }
    }
    return result_ids;
}

void PolygonSprite::setLineWidth(double w)
{
    line_property_->SetLineWidth(w);
    line_property_->Modified();
    modified();
}

// vtkAbstractPropPicker* PolygonSprite::getPicker()
// {
//     return picker_.Get();
// }

void PolygonSprite::setControlPointSelected(int index)
{
    if(selected_control_point_id_ == index){
        return;
    }
    selected_control_point_id_ = index;
    updatePolydata();
    modified();
}

Pt3Opt PolygonSprite::getSelectedPoint() const
{
    return getPoint(selected_control_point_id_);
}

// int PolygonSprite::getPickedControlPoint(Scene& s, int x, int y) const
// {
//     do{
//         if(picker_->GetCellId() == -1){
//             break;
//         }
//         auto picked_actor = picker_->GetActor();
//         if(picked_actor == nullptr){
//             break;
//         }
//         auto glyph_mapper = vtkGlyph3DMapper::SafeDownCast(picked_actor->GetMapper());
//         if(glyph_mapper == nullptr){
//             break;
//         }
//         Point3 picked_pos = picker_->GetPickPosition();
//         auto input_polys = vtkPolyData::SafeDownCast(glyph_mapper->GetInput());
//         if(input_polys == nullptr){
//             break;
//         }
//         vtkNew<vtkPointLocator> locator;
//         locator->SetDataSet(input_polys);
//         locator->BuildLocator();
//         return locator->FindClosestPoint(picked_pos);
//     }while(0);
//     return -1;
// }

NAMESPACE_END