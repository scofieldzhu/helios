#ifndef __vtkMeshVolumeScalarPicker_h
#define __vtkMeshVolumeScalarPicker_h

#include "vtkObject.h"
#include "helios/sprites/helios_sprites_export.h"

class vtkRenderer;
class vtkImageData;
class vtkTransform;

class HELIOS_SPRITES_API vtkMeshVolumeScalarPicker : public vtkObject
{
public:
	static vtkMeshVolumeScalarPicker *New();

	vtkTypeMacro(vtkMeshVolumeScalarPicker,vtkObject);
	void PrintSelf(ostream& os, vtkIndent indent);
	// 从屏幕坐标系转到volume上的pick点世界坐标系
	static bool Pick(double selectionX, double selectionY, double selectionZ, 
		vtkRenderer *renderer, vtkImageData * volume, double scalar, double pickedPoint[3]);
	static bool PickWithTransform(double selectionX, double selectionY, double selectionZ, 
		vtkRenderer *renderer, vtkImageData * volume, double scalar, vtkTransform *volumeTransform, double pickedPoint[3]);

	static bool PickOnRay(double rayPoint[3], double rayDir[3], vtkImageData * volume, double scalar, double pickedPoint[3]);
	static double LinearInterpolation(double point[3], double origin[3], double spaces[3], vtkIdType increment[3], short * colorPointer);
	static bool IsPointInsideBoundingBox(double point[3], double bounds[6]);

protected:
	vtkMeshVolumeScalarPicker();
	~vtkMeshVolumeScalarPicker();

private:

};
#endif