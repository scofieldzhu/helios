/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2024)
* author: zhucg
* time:2025/4/8
*******************************************************/
#ifndef __mirfak_navigation_typedef_h__
#define __mirfak_navigation_typedef_h__

#include "mirfak/core/mirfak_core_typedef.h"

MIRFAK_NAMESPACE_BEGIN

using QStringOpt = std::optional<QString>;
using Pose6d = std::array<double, 6>;
using Pose6dOpt = std::optional<Pose6d>;
using Matrix4x4d = std::array<double, 16>;
using CamPose = Matrix4x4d;

constexpr Pose6d NanPose = std::to_array({(double)NAN, (double)NAN, (double)NAN, (double)NAN, (double)NAN, (double)NAN});

enum class TrackerDeviceType
{
    kNone,
    kNDIVega,
    kAIM,
    kARMD
};

template <typename T>
concept EnumType = std::is_enum_v<T>;

template <EnumType T>
struct EnumTraits{};

template <>
struct EnumTraits<TrackerDeviceType>{
    static constexpr std::array<std::string_view, 4> names = {
        "None", "NDIVega", "AIM", "ARMD"
    };
    static constexpr std::string_view toString(TrackerDeviceType type){
        return names[static_cast<size_t>(type)];
    }
    static std::optional<TrackerDeviceType> fromString(std::string_view str){
        auto it = std::find(names.begin(), names.end(), str);
        if(it != names.end()){
            return static_cast<TrackerDeviceType>(it - names.begin());
        }
        return std::nullopt;
    }
};

enum class TrackerToolType
{
    kNone,
    kLocator,
    kReferencer,
    kProbe
};

template <>
struct EnumTraits<TrackerToolType>{
    static constexpr std::array<std::string_view, 4> names = {
        "None", "Locator", "Referencer", "Probe"
    };
    static constexpr std::string_view toString(TrackerToolType type){
        return names[static_cast<size_t>(type)];
    }
    static std::optional<TrackerToolType> fromString(std::string_view str){
        auto it = std::find(names.begin(), names.end(), str);
        if(it != names.end()){
            return static_cast<TrackerToolType>(it - names.begin());
        }
        return std::nullopt;
    }
};

enum class TrackerToolStatus
{
    kNone,
    kMissing,	
    kOutOfView, 
    kNormal
};

constexpr double kDefaultDrillLength = 10.0;
constexpr double kDefaultCamPosLength = 5.0;

constexpr int kNullToolPort = -1;

inline bool IsValidToolPort(int port){
    return port > 0;
}

constexpr double kInvalidToolError = -100.0;
inline bool IsInvalidToolError(double err){
    return err < 0.0;
}

NAMESPACE_END

#endif
