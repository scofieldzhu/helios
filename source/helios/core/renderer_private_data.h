/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/6/20
*******************************************************/
#ifndef __renderer_private_data_h__
#define __renderer_private_data_h__

#include <vtkObject.h>
#include "helios/core/core_pre_decl.h"

class RendererPrivateData : public vtkObject 
{
public:
    vtkTypeMacro(RendererPrivateData, vtkObject);
	static RendererPrivateData* New();
	helios::Scene* scene = nullptr;
    
private:
	RendererPrivateData(){}
	~RendererPrivateData(){}
};

#endif