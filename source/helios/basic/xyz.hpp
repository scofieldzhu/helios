/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/1/24
*******************************************************/
#ifndef __xyz_hpp__
#define __xyz_hpp__

#include <stdexcept>
#include <sstream>
#include <numbers>
#include <array>
#include <vector>
#include <optional>
#include "helios/basic/langext.h"

HELIOS_NAMESPACE_BEGIN

namespace detail{
	template <typename T>
	concept IsXYZArg = IsFloating<T> || IsIntegral<T>;
}

template <detail::IsXYZArg T>
class XYZ
{
public:
    T d[3];

	void zero(){
		d[0] = T(0);
		d[1] = T(0);
		d[2] = T(0);
	}

    void setX(T v){
        d[0] = v;
    }

    T x()const{
        return d[0];
    }

    void setY(T v){
        d[1] = v;
    }

    T y()const{
        return d[1];
    }

    void setZ(T v){
        d[2] = v;
    }

    T z()const{
        return d[2];
    }

    operator T*() {
        return d;
    }

    operator const T*()const{
        return d;
    }

	std::array<T, 3> stdArray()const{
		return std::to_array(d);
	}

	bool isNan()const requires detail::IsFloating<T>{
		return isnan(x()) || isnan(y()) || isnan(z());
	}

	constexpr T squareNorm()const{
		return x() * x() + y() * y() + z() * z(); 
	}

	T norm()const requires detail::IsFloating<T>{
		return std::sqrt(x() * x() + y() * y() + z() * z()); 
	}

	T length()const requires detail::IsFloating<T>{
		return norm();
	}

	void normalize() requires detail::IsFloating<T>{
		operator/=(norm());
	}

	XYZ normalized()const requires detail::IsFloating<T>{
		auto self = *this;
		self.normalize();
		return self;
	}	

	constexpr T dot(const XYZ& rhs) const{  
    	return x() * rhs.x() + y() * rhs.y() + z() * rhs.z();  
	}

	constexpr void cross(const XYZ& rhs){
        auto Dx = y() * rhs.z() - z() * rhs.y();
        auto Dy = z() * rhs.x() - x() * rhs.z();
        auto Dz = x() * rhs.y() - y() * rhs.x();
		setX(Dx);
		setY(Dy);
		setZ(Dz);
	}

	constexpr XYZ crossed(const XYZ& rhs) const{  
		auto self = *this;
		self.cross(rhs);
		return self;
	}

	constexpr XYZ cross(const XYZ& rhs) const{  
		return crossed(rhs);
	}  

	constexpr XYZ centerTo(const XYZ& rhs)const{
		return (*this + rhs) / T{2};
	}

	T distanceTo(const XYZ& rhs)const requires detail::IsFloating<T>{
		return (rhs - *this).norm();
	}

	T angle(const XYZ& rhs)const requires detail::IsFloating<T>{
		auto val = norm() * rhs.norm();
		if(IsZero(val)){
			val = kZeroTolerance;
		}			
		auto d_val = dot(rhs) / val;
		return 180.0 * acos(d_val) / std::numbers::pi;
	}

	constexpr XYZ operator+(const XYZ& rhs)const{
		return {x() + rhs.x(), y() + rhs.y(), z() + rhs.z()};
	}

	constexpr XYZ& operator+=(const XYZ& rhs){
		d[0] += rhs.x();
		d[1] += rhs.y();
		d[2] += rhs.z();
		return *this;
	}

	constexpr XYZ operator-()const{
		return {-x(), -y(), -z()};
	}

	constexpr XYZ operator-(const XYZ& rhs)const{
		return {x() - rhs.x(), y() - rhs.y(), z() - rhs.z()};
	}

	constexpr XYZ& operator-=(const XYZ& rhs){
		d[0] -= rhs.x;
		d[1] -= rhs.y;
		d[2] -= rhs.z;
		return *this;
	}

	constexpr XYZ operator*(T scalar)const{
		return {x() * scalar, y() * scalar, z() * scalar};
	}

	constexpr XYZ& operator*=(T scalar){
		d[0] *= scalar;
		d[1] *= scalar;
		d[2] *= scalar;
		return *this;
	}

	XYZ operator/(T scalar) const{
		if(IsZero(scalar)){
			throw std::runtime_error("Division by zero in XY.");
		}
		return {x() / scalar, y() / scalar, z() / scalar};
	}

	bool operator==(const XYZ& rhs)const{
		return IsZero(x() - rhs.x()) && IsZero(y() - rhs.y()) && IsZero(z() - rhs.z());
	}

	bool operator!=(const XYZ& rhs)const{
		return !operator==(rhs);
	}	

	XYZ& operator/=(T scalar){
		return operator=(const_cast<const XYZ&>(*this) / scalar);
	}

	//std::string toStr(char separator = ',')const requires detail::IsIntegral<T>{
	//	std::stringstream ss;
	//	ss << '(' << x() << separator << y() << separator << z() << ')';
	//	return ss.str();
	//}

	std::string toStr(char separator = ',', int precision = 3)const /*requires detail::IsFloating<T>*/{
		std::stringstream ss;
		ss.setf(std::ios::fixed, std::ios::floatfield);
		ss.precision(precision);
		ss << '(' << x() << separator << y() << separator << z() << ')';
		return ss.str();
	}

	friend std::ostream& operator<<(std::ostream& os, const XYZ& xyz){  
		os << "(" << xyz.x() << ", " << xyz.y() << ", " << xyz.z() << ")";  
		return os;  
	}

	XYZ& operator=(const T* data){
		d[0] = data[0];
		d[1] = data[1];
		d[2] = data[2];
		return *this;
	}

	constexpr XYZ()
		:d{T(0), T(0), T(0)}
    {}

	constexpr XYZ(T v_x, T v_y, T v_z)
		:d{v_x, v_y, v_z}
    {}

	constexpr XYZ(const T v[3])
		:d{v[0], v[1], v[2]}
    {}

	XYZ& operator=(const XYZ&) = default;
	constexpr XYZ(const XYZ&) = default;
	XYZ(XYZ&&) = default;
	~XYZ() = default;
};

//Some commonly used template classes are instantiated based on XYZ:
using Point3  = XYZ<double>;
using Pt3List = std::vector<Point3>;
using Point3u = XYZ<unsigned int>;
using Pt3uList = std::vector<Point3u>;
using Point3i = XYZ<int>;
using Pt3iList = std::vector<Point3i>;
using Vec3    = XYZ<double>;
using Vec3List = std::vector<Vec3>;

using Pt3Arry6 = std::array<Point3, 6>;
using Pt3Arry5 = std::array<Point3, 5>;
using Pt3Arry4 = std::array<Point3, 4>;
using Pt3Arry3 = std::array<Point3, 3>;
using Pt3Arry2 = std::array<Point3, 2>;

using Pt3iArry6 = std::array<Point3i, 6>;
using Pt3iArry5 = std::array<Point3i, 5>;
using Pt3iArry4 = std::array<Point3i, 4>;
using Pt3iArry3 = std::array<Point3i, 3>;
using Pt3iArry2 = std::array<Point3i, 2>;

using Pt3Opt = std::optional<Point3>;
using Vec3Opt = std::optional<Vec3>;

NAMESPACE_END

#endif