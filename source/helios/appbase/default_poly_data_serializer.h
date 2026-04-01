/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/25
*******************************************************/
#ifndef __default_poly_data_serializer_h__
#define __default_poly_data_serializer_h__

#include "vtk_poly_data_serializer.h"
#include "helios/appbase/helios_appbase_export.h"

HELIOS_NAMESPACE_BEGIN

class HELIOS_APPBASE_API DefaultPolyDataSerializer : public VtkPolyDataSerializer
{
public:
	bool writeFile(vtkPolyData* data, const QString& filepath) override;
	vtkSmartPointer<vtkPolyData> readFile(const QString& filepath) override;

private:
	static void OnCallback(vtkObject* caller, unsigned long eid, void* clientdata, void* calldata);
	void handleCallback(vtkObject* caller, unsigned long e_id, void* call_data);
};

NAMESPACE_END

#endif