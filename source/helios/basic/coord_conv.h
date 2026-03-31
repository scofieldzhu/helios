/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/3/8
*******************************************************/
#ifndef __coord_conv_h__
#define __coord_conv_h__

#include <vtkSmartPointer.h>
#include "mirfak/basic/mirfak_basic_typedef.h"
#include "mirfak/basic/mirfak_basic_export.h"

class vtkPoints;

MIRFAK_NAMESPACE_BEGIN

MIRFAK_BASIC_API vtkSmartPointer<vtkPoints> ToVtkPoints(const Pt3List& pts);
MIRFAK_BASIC_API Pt3List FromVtkPoints(vtkPoints* pts);

NAMESPACE_END

#endif