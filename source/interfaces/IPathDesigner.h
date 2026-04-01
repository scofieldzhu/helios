
/******************************************************** 
* author: scofieldzhu
* time:2025/11/24
*******************************************************/
#ifndef __IPathDesigner_h__
#define __IPathDesigner_h__

#include <QObject>
#include "helios/sprites/surgical_path_sprite.h"

class IPathDesigner
{
public:
    virtual void loadPaths(helios::SurgicalPathGroup& gp) = 0;
    virtual helios::SurgicalPathGroup* getPathGroup() = 0;
    virtual ~IPathDesigner() = default;    
};

#define IPATH_DESIGNER_IID "helios.plugin.IPathDesigner/1.0"

Q_DECLARE_INTERFACE(IPathDesigner, IPATH_DESIGNER_IID)

#endif