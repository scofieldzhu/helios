/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/5/15
*******************************************************/
#include "backdrop_sprite.h"
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkCoordinate.h>
#include <vtkActor2D.h>
#include <vtkProperty2D.h>

MIRFAK_NAMESPACE_BEGIN

BackdropSprite::BackdropSprite()
    :polys_(vtkSmartPointer<vtkPolyData>::New())
{
}

BackdropSprite::~BackdropSprite()
{
}

void BackdropSprite::createSource(Scene& scene)
{
    vtkNew<vtkPoints> pts;
    pts->InsertNextPoint(0, 0, 0);   // 左下 (归一化 0-1)
    pts->InsertNextPoint(1, 0, 0);   // 右下
    pts->InsertNextPoint(1, 1, 0);   // 右上
    pts->InsertNextPoint(0, 1, 0);   // 左上
    vtkNew<vtkCellArray> cell_array;
    vtkIdType pt_indexes[4] = {0, 1, 2 ,3};
    cell_array->InsertNextCell(4, pt_indexes);
    polys_->SetPoints(pts);
    polys_->SetPolys(cell_array);
}

void BackdropSprite::makeActors(Scene& scene)
{
    vtkNew<vtkPolyDataMapper2D> mapper;
    mapper->SetInputData(polys_);
    vtkNew<vtkCoordinate> coord;
    coord->SetCoordinateSystemToNormalizedViewport();
    mapper->SetTransformCoordinate(coord);
    vtkNew<vtkActor2D> actor;
    actor->SetMapper(mapper);
    addSceneProp(scene, actor, "backdrop");
    actor->GetProperty()->SetDisplayLocationToBackground();
    actor->PickableOff();
}

NAMESPACE_END
