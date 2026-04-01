/******************************************************** 
* author: scofieldzhu
* time:2025/12/25
*******************************************************/
#ifndef __vtk_image_data_serializer_h__
#define __vtk_image_data_serializer_h__

#include <QString>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include "helios/basic/signal.hpp"
#include "helios/helios_nsp.h"

HELIOS_NAMESPACE_BEGIN

class VtkImageDataSerializer
{
public:
	Signal<double, QString> Progress;
	virtual bool writeFile(vtkImageData* data, const QString& filepath) = 0;
	virtual vtkSmartPointer<vtkImageData> readFile(const QString& filepath) = 0;
	virtual ~VtkImageDataSerializer() = default;
};

NAMESPACE_END

#endif