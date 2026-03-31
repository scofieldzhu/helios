/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/25
*******************************************************/
#ifndef __vtk_poly_data_serializer_h__
#define __vtk_poly_data_serializer_h__

#include <QString>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include "mirfak/basic/signal.hpp"
#include "mirfak/mirfak_nsp.h"

MIRFAK_NAMESPACE_BEGIN

class VtkPolyDataSerializer
{
public:
	Signal<double, QString> Progress;
	virtual bool writeFile(vtkPolyData* data, const QString& filepath) = 0;
	virtual vtkSmartPointer<vtkPolyData> readFile(const QString& filepath) = 0;
	virtual ~VtkPolyDataSerializer() = default;
};

NAMESPACE_END

#endif