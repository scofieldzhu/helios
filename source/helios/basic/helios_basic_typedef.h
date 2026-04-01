/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/1/24
*******************************************************/
#ifndef __helios_basic_typedef_h__
#define __helios_basic_typedef_h__

#include "helios/basic/xy.hpp"
#include "helios/basic/xyz.hpp"
#include "helios/basic/color.h"
#include "helios/basic/line.h"
#include "helios/basic/plane.h"
#include "helios/basic/bound_plane.h"

HELIOS_NAMESPACE_BEGIN

using Arry6d = std::array<double, 6>;
using Arry5d = std::array<double, 5>;
using Arry4d = std::array<double, 4>;
using Arry3d = std::array<double, 3>;
using Arry2d = std::array<double, 2>;

using Arry6i = std::array<int, 6>;
using Arry5i = std::array<int, 5>;
using Arry4i = std::array<int, 4>;
using Arry3i = std::array<int, 3>;
using Arry2i = std::array<int, 2>;

template<typename T = double, size_t N = 3>
std::array<T, N> ToArray(const T* p){
	std::array<T, N> a{};
	std::copy_n(p, N, a.begin());
	return a;
}

using ByteBuffer = std::vector<uint8_t>;

NAMESPACE_END

#endif
