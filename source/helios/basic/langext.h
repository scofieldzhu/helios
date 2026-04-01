/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/3/1
*******************************************************/
#ifndef __langext_h__
#define __langext_h__

#include <cmath>
#include <concepts>
#include <numbers>
#include <type_traits>
#include "helios/helios_nsp.h"

HELIOS_NAMESPACE_BEGIN

constexpr float kZeroTolerance = 1e-5f;

namespace detail
{
    template <typename T>
	concept IsFloating = std::is_floating_point_v<T>;
	template <typename T>
	concept IsIntegral = std::is_integral_v<T>;
}

template <typename Type>
inline bool IsZero(Type v)
{
    if constexpr(detail::IsFloating<Type>){
        return fabs(v) <= kZeroTolerance;
    }else{
        return v == Type{0};
    }    
}

// verify that types are complete for increased safety
template<class T> inline void CheckDelete(T * x)
{
    // intentionally complex - simplification causes regressions
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    delete x;
}

template<class T> inline void CheckArrayDelete(T * x)
{
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    delete[] x;
}

template<class T> struct VXDeleter
{
    typedef void result_type;
    typedef T * argument_type;

    void operator()(T * x) const
    {
        // boost:: disables ADL
        CheckDelete(x);
    }
};

template<class T> struct VXArryDeleter
{
    typedef void result_type;
    typedef T * argument_type;

    void operator()(T * x) const
    {
        CheckArrayDelete(x);
    }
};

template <class T>
constexpr T RadianToDegree(T rad)
{
    static_assert(std::is_floating_point_v<T>, "use a floating-point type");
    return rad * (T(180) / std::numbers::pi_v<T>);
}

template <class T>
constexpr T DegreeToRadiant(T deg)
{
    static_assert(std::is_floating_point_v<T>, "use a floating-point type");
    return deg * (std::numbers::pi_v<T> / T(180));
}

NAMESPACE_END

#endif