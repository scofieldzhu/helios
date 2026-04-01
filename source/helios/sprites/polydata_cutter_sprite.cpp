/*******************************************************
* author: scofieldzhu
* time:2025/9/12
*******************************************************/
#include "polydata_cutter_sprite.h"
#include <vtkCutter.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCleanPolyData.h>
#include <vtkPlane.h>
#include <vtkTransform.h>

HELIOS_NAMESPACE_BEGIN
PolyDataCutterSprite::PolyDataCutterSprite()
    :cutter_(vtkSmartPointer<vtkCutter>::New()),
    clean_filter_(vtkSmartPointer<vtkCleanPolyData>::New()),
    trans_filter_(vtkSmartPointer<vtkTransformPolyDataFilter>::New())
{

}

PolyDataCutterSprite::~PolyDataCutterSprite()
{

}

void PolyDataCutterSprite::setInputPolyData(vtkAlgorithmOutput* output_polys)
{
    trans_filter_->SetInputConnection(output_polys);
    cutter_->SetInputConnection(trans_filter_->GetOutputPort());
    cutter_->Modified();
}

void PolyDataCutterSprite::setPolyDataTransform(vtkTransform* trans)
{
    trans_filter_->SetTransform(trans);
    trans_filter_->Modified();
}

void PolyDataCutterSprite::setCutPlaneEquation(vtkPlane* plane)
{
    if(plane){
        cutter_->SetCutFunction(plane);
        cutter_->Modified();
    }
}

void PolyDataCutterSprite::createSource(Scene& scene)
{
    clean_filter_->SetInputConnection(cutter_->GetOutputPort());
}

void PolyDataCutterSprite::makeActors(Scene& scene)
{
    vtkNew<vtkPolyDataMapper> m;
    m->SetInputConnection(clean_filter_->GetOutputPort());
    vtkNew<vtkActor> a;
    a->SetMapper(m);
    addSceneProp(scene, a, "cutter");
}

NAMESPACE_END