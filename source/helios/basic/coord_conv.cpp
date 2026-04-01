/******************************************************** 
* author: scofieldzhu
* time:2025/3/8
*******************************************************/
#include "coord_conv.h"
#include <vtkPoints.h>

HELIOS_NAMESPACE_BEGIN

vtkSmartPointer<vtkPoints> ToVtkPoints(const Pt3List& pts)
{
    vtkNew<vtkPoints> vtk_pts;
    for(const auto& pt : pts){
        vtk_pts->InsertNextPoint(pt);
    }
    return vtk_pts;
}

Pt3List FromVtkPoints(vtkPoints* pts)
{
    Pt3List res_pts;
    if(pts){
        for(auto i = 0; i < pts->GetNumberOfPoints(); ++i){
            res_pts.push_back(pts->GetPoint(i));
        }
    }
    return res_pts;
}

NAMESPACE_END