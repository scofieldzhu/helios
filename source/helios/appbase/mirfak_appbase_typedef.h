/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2025/12/25
*******************************************************/
#ifndef __mirfak_appbase_typedef_h__
#define __mirfak_appbase_typedef_h__

#include "mirfak/sprites/mirfak_sprites_typedef.h"
#include "mirfak/appbase/log_misc.h"

MIRFAK_NAMESPACE_BEGIN

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