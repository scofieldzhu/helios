/******************************************************** 
* author: scofieldzhu
* time:2025/12/25
*******************************************************/
#ifndef __vtk_poly_data_serializer_h__
#define __vtk_poly_data_serializer_h__

#include <QString>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include "helios/basic/signal.hpp"
#include "helios/helios_nsp.h"

HELIOS_NAMESPACE_BEGIN

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