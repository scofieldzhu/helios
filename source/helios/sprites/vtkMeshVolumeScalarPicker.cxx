/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkMeshVolumeScalarPicker.cxx,v $

Copyright (c) Luo Zhe Digital Health Care
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMeshVolumeScalarPicker.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkRenderer.h"
#include "vtkMath.h"
#include "vtkCamera.h"
#include "vtkTransform.h"


using namespace std;
vtkStandardNewMacro(vtkMeshVolumeScalarPicker);


//----------------------------------------------------------------------------
vtkMeshVolumeScalarPicker::vtkMeshVolumeScalarPicker()
{
	
}
//----------------------------------------------------------------------------
vtkMeshVolumeScalarPicker::~vtkMeshVolumeScalarPicker()
{
	
}

bool vtkMeshVolumeScalarPicker::Pick(double selectionX, double selectionY, double selectionZ, 
	vtkRenderer *renderer, vtkImageData * volume, double scalar, double pickedPoint[3])
{
	renderer->SetDisplayPoint(selectionX, selectionY, selectionZ);
	renderer->DisplayToWorld();
	double * worldCoords = renderer->GetWorldPoint();
	if ( worldCoords[3] == 0.0 )
	{
		cout <<"Bad homogeneous coordinates";
		return false;
	}

	double cameraPos[4],  cameraFP[4];
	vtkCamera * camera = renderer->GetActiveCamera();
	camera->GetPosition(cameraPos);
	cameraPos[3] = 1.0;
	camera->GetFocalPoint(cameraFP);
	cameraFP[3] = 1.0;

	double rayPoint[3], rayDir[3];
	for (int i=0; i < 3; i++) 
		rayPoint[i] = worldCoords[i] / worldCoords[3];
	for (int i = 0; i < 3; i++)
		rayDir[i] = cameraFP[i] - cameraPos[i];
	return PickOnRay(rayPoint, rayDir, volume, scalar, pickedPoint);
}

bool vtkMeshVolumeScalarPicker::PickWithTransform(double selectionX, double selectionY, double selectionZ, 
	vtkRenderer *renderer, vtkImageData * volume, double scalar, vtkTransform *volumeTransform, double pickedPoint[3])
{
	renderer->SetDisplayPoint(selectionX, selectionY, selectionZ);
	renderer->DisplayToWorld();
	double * worldCoords = renderer->GetWorldPoint();
	if ( worldCoords[3] == 0.0 )
	{
		cout <<"Bad homogeneous coordinates";
		return false;
	}

	vtkTransform * inverseTransform = vtkTransform::New();
	inverseTransform->Identity();

	if (volumeTransform)
	{
		inverseTransform->GetMatrix()->DeepCopy(volumeTransform->GetMatrix());
		inverseTransform->Inverse();
		inverseTransform->Update();
	}

	double cameraPos[4],  cameraFP[4];
	vtkCamera * camera = renderer->GetActiveCamera();
	camera->GetPosition(cameraPos);
	cameraPos[3] = 1.0;
	camera->GetFocalPoint(cameraFP);
	cameraFP[3] = 1.0;

	double rayPoint[3], rayDir[3];
	for (int i=0; i < 3; i++) 
		rayPoint[i] = worldCoords[i] / worldCoords[3];
	for (int i = 0; i < 3; i++)
		rayDir[i] = cameraFP[i] - cameraPos[i];

	// transform 
	double rayPointT[3];
	double rayDirT[3];
	inverseTransform->TransformPoint(rayPoint, rayPointT);
	inverseTransform->TransformNormal(rayDir, rayDirT);

	bool resultTag = PickOnRay(rayPointT, rayDirT, volume, scalar, pickedPoint);
	double pickedPointT[3];
	volumeTransform->TransformPoint(pickedPoint, pickedPointT);
	for (int i = 0; i < 3; i++)
		pickedPoint[i] = pickedPointT[i];
	inverseTransform->Delete();

	return resultTag;
}
//    ݹ           Ͷ 䣬ѡ   һ      scalar  ֵ  Ϊʰȡ  
bool vtkMeshVolumeScalarPicker::PickOnRay(double rayPoint[3], double rayDir[3], vtkImageData * volume, 
	double scalar, double pickedPoint[3])
{
	double stepLen = 0.1;
	vtkMath::Normalize(rayDir);
	int dimension[3]; 
	volume->GetDimensions(dimension);
	double spacing[3];
	volume->GetSpacing(spacing);
	double bounds[6];
	volume->GetBounds(bounds);
	double origin[3];
	volume->GetOrigin(origin);
	vtkIdType increment[3];
	volume->GetIncrements(increment);
	short * colorPointer = (short *)volume->GetScalarPointer();

	double maxSegmentLen;
	double minSegmentLen;
	double minAxisLength = VTK_DOUBLE_MAX;

	for (int k = 0; k < 3; k++)
	{
		if (abs( (bounds[2 * k + 1] - bounds[2 * k]) / rayDir[k]) < minAxisLength)
		{
			minAxisLength = abs( (bounds[2 * k + 1] - bounds[2 * k] + 0.002) / rayDir[k]);
			maxSegmentLen =  (bounds[2 * k + 1] - rayPoint[k] + 0.001) / rayDir[k];
			minSegmentLen =  (bounds[2 * k] - rayPoint[k] - 0.001) / rayDir[k];
		}			
	}
	double startPos[3];
	if (maxSegmentLen > minSegmentLen)
	{
		for (int k = 0; k < 3; k++)
			startPos[k] = rayPoint[k] + minSegmentLen * rayDir[k];
	
	}
	else
	{
		for (int k = 0; k < 3; k++)
			startPos[k] = rayPoint[k] + maxSegmentLen * rayDir[k];
	}

	double totalLen = minAxisLength;

	const int vertMap[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}} ;

	double currentT = 0;
	double leftPos[3] = {startPos[0], startPos[1], startPos[2]};
	double rightPos[3];
	for (int i = 0; i < 3; i++)
		rightPos[i] = leftPos[i] + stepLen * rayDir[i];
	double leftValue, rightValue;

	bool existTag = false;
	while (currentT < totalLen)
	{
		if (IsPointInsideBoundingBox(leftPos,  bounds) && IsPointInsideBoundingBox(rightPos, bounds))
		{
			leftValue = LinearInterpolation(leftPos, origin, spacing, increment, colorPointer);
			rightValue = LinearInterpolation(rightPos, origin, spacing, increment, colorPointer);

			if (rightValue - scalar > 0)
			{
				existTag = true;	
				break;
			}
		}

		currentT += stepLen;
		for (int k = 0; k < 3; k++)
		{
			leftPos[k] = rightPos[k];
			rightPos[k] = leftPos[k] + rayDir[k] * stepLen;
		}
	}

	if (existTag)
	{
		for (int k = 0; k < 3; k++)
			pickedPoint[k] = (leftPos[k] + rightPos[k]) / 2;
		return true;
	}
	else
	{
		pickedPoint[0] = 0.0;
		pickedPoint[1] = 0.0;
		pickedPoint[2] = 0.0;
		return false;
	}
}


double vtkMeshVolumeScalarPicker::LinearInterpolation(double point[3], double origin[3], double spaces[3], 
	vtkIdType increment[3], short * colorPointer)
{
	int leftPos[3];
	float volumePos[3];

	for (int i = 0; i < 3; i++)
	{
		volumePos[i] = (point[i] - origin[i]) / spaces[i];
		leftPos[i] = int(volumePos[i]);
	}

	float          A, B, C, D, E, F, G, H;
	int             Binc, Cinc, Dinc, Einc, Finc, Ginc, Hinc;
	int             xinc, yinc, zinc;
	float          x, y, z, t1, t2, t3;
	int             offset;
	short*        dptr;
	float          scalar;

	xinc = increment[0];
	yinc = increment[1];
	zinc = increment[2];

	offset = leftPos[2] * zinc + leftPos[1] * yinc + leftPos[0];
	dptr = colorPointer + offset;

	Binc = xinc;
	Cinc = yinc;
	Dinc = xinc + yinc;
	Einc = zinc;
	Finc = zinc + xinc;
	Ginc = zinc + yinc;
	Hinc = zinc + xinc + yinc;

	A = *(dptr);
	B = *(dptr + Binc);
	C = *(dptr + Cinc);
	D = *(dptr + Dinc);
	E = *(dptr + Einc);
	F = *(dptr + Finc);
	G = *(dptr + Ginc);
	H = *(dptr + Hinc);

	// Compute our offset in the voxel, and use that to trilinearly
	// interpolate the value
	x = volumePos[0] - (float) leftPos[0];
	y = volumePos[1] - (float) leftPos[1];
	z = volumePos[2] - (float) leftPos[2];

	t1 = 1.0 - x;
	t2 = 1.0 - y;
	t3 = 1.0 - z;

	scalar = 
		A * t1 * t2 * t3 +
		B *  x * t2 * t3 +
		C * t1 *  y * t3 + 
		D *  x *  y * t3 +
		E * t1 * t2 *  z + 
		F *  x * t2 *  z + 
		G * t1 *  y *  z + 
		H *  x *  y *  z;
	return scalar;
}

bool vtkMeshVolumeScalarPicker::IsPointInsideBoundingBox(double point[3], double bounds[6])
{
	for (int i = 0; i < 3; i++)
	{
		if (point[i] < bounds[2 * i] || point[i] > bounds[2 * i + 1])
			return false;
	}
	return true;
}

//----------------------------------------------------------------------------
void vtkMeshVolumeScalarPicker::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtkMeshVolumeScalarPicker\n";
}
