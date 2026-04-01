/*******************************************************
* author: scofieldzhu
* time:2025/8/26
*******************************************************/
#ifndef __abstract_volume_sprite_h__
#define __abstract_volume_sprite_h__

#include "helios/core/sprite.h"
#include "helios/sprites/helios_sprites_export.h"

class vtkImageData;
class vtkVolumeProperty;
class vtkProp3D;
class vtkOpenGLGPUVolumeRayCastMapper;
class vtkPlane;
class vtkRenderer;

HELIOS_NAMESPACE_BEGIN

class HELIOS_SPRITES_API AbstractVolumeSprite: public Sprite
{  
	SPRITE_DECL(AbstractVolumeSprite, Sprite)
public:        
	// PickInfoList getPickList(vtkCellPicker* picker, Scene& pane, int x, int y) const override;
    // bool getPickPosition(vtkCellPicker* picker, Scene& pane, const Point2u& pos, vtkImageData* maskdata, Point3* pickedpt, int* pickedintensity);
    bool hasChangedSince(unsigned long since_mtime) const override;
    virtual void setInputData(vtkImageData* input);
    vtkImageData* getInput()const;
    void setMaskImageData(vtkImageData* maskdata);
    vtkImageData* maskImageData()const;
    virtual vtkProp3D* getVolume() = 0;
    virtual vtkVolumeProperty* getVolumeProperty()const = 0;   
    void turnClippingPlaneOn();
    void turnClippingPlaneOff();
    bool isClippingPlaneOn()const;
    void updateClippingPlane(const Point3& origin, const Vec3& normal);
    virtual Pt3Opt pickWorldPoint(int x, int y, Scene& s) = 0;
	AbstractVolumeSprite(vtkIdType max_mem_bytes);
    ~AbstractVolumeSprite();

protected:
    void onStartEventNotify(vtkObject* caller, unsigned long event_id, void* data);
    bool addToScene(Scene& scene) override;
    void removeFromScene(Scene& scene) override;
    vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> raycast_mapper_;
    unsigned long start_render_event_id_ = -1;
    vtkRenderer* event_renderer_ = nullptr;
    vtkSmartPointer<vtkPlane> clipping_plane_;
};

NAMESPACE_END

#endif
