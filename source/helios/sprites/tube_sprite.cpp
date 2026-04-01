/*******************************************************
* author: scofieldzhu
* time:2025/9/16
*******************************************************/
#include "tube_sprite.h"
#include <vtkTubeFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkCleanPolyData.h>
#include <vtkProperty.h>

HELIOS_NAMESPACE_BEGIN

TubeSprite::TubeSprite(double r)
    :tube_source_(vtkSmartPointer<vtkTubeFilter>::New()),
    clean_polydata_(vtkSmartPointer<vtkCleanPolyData>::New())
{
    tube_source_->SetRadius(r);
    tube_source_->SetCapping(1);
    tube_source_->SetNumberOfSides(8);
}

TubeSprite::~TubeSprite()
{
}

void TubeSprite::setPoints(const Pt3List& pts)
{    
    vtkNew<vtkPolyData> polys;
    vtkNew<vtkPoints> vtk_points;
    for(auto p : pts){
        vtk_points->InsertNextPoint(p);
    }
    polys->SetPoints(vtk_points);
    vtkNew<vtkCellArray> lines;
    vtkIdType ids[2] = {0};
    for(auto i = 1; i < vtk_points->GetNumberOfPoints(); ++i){
        ids[0] = i - 1;
        ids[1] = i ;
        lines->InsertNextCell(2, ids);
    }
    polys->SetLines(lines);
    tube_source_->SetInputData(polys);    
    tube_source_->Modified();
    modified();
}

Pt3List TubeSprite::getPoints()const
{
    Pt3List pts;
    auto plys = vtkPolyData::SafeDownCast(tube_source_->GetInputDataObject(0, 0));
    if(plys){
        for(auto i = 0; i < plys->GetNumberOfPoints(); ++i){
            Point3 pt = plys->GetPoint(i);
            pts.push_back(pt);
        }
    }
    return pts;
}

void TubeSprite::setSourcePolys(vtkPolyData* ply)
{
    if(ply){
        tube_source_->SetInputData(ply);
        tube_source_->Modified();
        modified();
    }    
}

void TubeSprite::makeActors(Scene& scene)
{
    vtkNew<vtkPolyDataMapper> m;
    m->SetInputConnection(clean_polydata_->GetOutputPort());
    vtkNew<vtkActor> a;
    a->SetMapper(m);
    addSceneProp(scene, a, "tube");
}

void TubeSprite::createSource(Scene& scene)
{
    clean_polydata_->SetInputConnection(tube_source_->GetOutputPort());
}

NAMESPACE_END
