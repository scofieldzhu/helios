/******************************************************** 
* author: scofieldzhu
* time:2025/3/8
*******************************************************/
#ifndef __coord_conv_h__
#define __coord_conv_h__

#include <vtkSmartPointer.h>
#include "helios/basic/helios_basic_typedef.h"
#include "helios/basic/helios_basic_export.h"

class vtkPoints;

HELIOS_NAMESPACE_BEGIN

HELIOS_BASIC_API vtkSmartPointer<vtkPoints> ToVtkPoints(const Pt3List& pts);
HELIOS_BASIC_API Pt3List FromVtkPoints(vtkPoints* pts);

NAMESPACE_END

#endif