/******************************************************** 
* author: scofieldzhu
* time:2025/12/25
*******************************************************/
#ifndef __helios_appbase_typedef_h__
#define __helios_appbase_typedef_h__

#include "helios/sprites/helios_sprites_typedef.h"
#include "helios/appbase/log_misc.h"

HELIOS_NAMESPACE_BEGIN

class Session;
class SessionContext;
class SessionManager;
using SessionPtr = std::shared_ptr<Session>;
using SessionUPtr = std::unique_ptr<Session>;

class VtkImageDataSerializer;
class DefaultImageDataSerializer;
class VtkPolyDataSerializer;
class DefaultPolyDataSerializer;

class DocPackage;
class DocFileSerializer;

class PluginManager;

class SceneLayout;
class SceneLayoutManager;

struct DICOMMetaData;

class ProgressControlView;

NAMESPACE_END

#endif