/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/2/9
*******************************************************/
#ifndef __mirfak_core_typedef_h__
#define __mirfak_core_typedef_h__

#include <memory>
#include <functional>
#include "mirfak/basic/mirfak_basic_typedef.h"
#include "mirfak/core/core_pre_decl.h"

MIRFAK_NAMESPACE_BEGIN

using SpriteSPtr = std::shared_ptr<Sprite>;
using SpriteSList = std::vector<SpriteSPtr>;
using SpriteUPtr = std::unique_ptr<Sprite>;
using SpriteList = std::vector<Sprite*>;
using ConstSpriteList = std::vector<const Sprite*>;

using SceneSPtr = std::shared_ptr<Scene>;
using SceneUPtr = std::unique_ptr<Scene>;
using SceneList = std::vector<Scene*>;

using EditorPtr = std::shared_ptr<Editor>;

inline constexpr std::string_view Axial = "Axial";
inline constexpr std::string_view Coronal = "Coronal";
inline constexpr std::string_view Sagittal = "Sagittal";
inline constexpr std::string_view Ortho = "Ortho";

NAMESPACE_END

#endif