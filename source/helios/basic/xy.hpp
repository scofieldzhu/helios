/******************************************************** 
* author: scofieldzhu
* time:2025/1/19
*******************************************************/
#ifndef __xy_hpp__
#define __xy_hpp__

#include <stdexcept>
#include <sstream>
#include <vector>
#include "helios/basic/langext.h"

HELIOS_NAMESPACE_BEGIN

namespace detail{
	template <typename T>
	concept IsXYArg = IsFloating<T> || IsIntegral<T>;
}

template <detail::IsXYArg T>
class XY
{
public:
	T x, y;

	constexpr bool isNan()const requires detail::IsFloating<T>{
		return isnan(x) || isnan(y);
	}

	constexpr T squareNorm()const{
		return x * x + y * y; 
	}

	T distanceTo(const XY& rhs)const requires detail::IsFloating<T>{
		return ((*this) - rhs).norm();
	}

	T norm()const requires detail::IsFloating<T>{
		return std::sqrt(x * x + y * y); 
	}

	T length()const requires detail::IsFloating<T>{
		return norm();
	}

	void normalize() requires detail::IsFloating<T>{
		operator/=(norm());
	}

	XY normalized()const requires detail::IsFloating<T>{
		auto self = *this;
		self.normalize();
		return self;
	}	

	constexpr T dot(const XY& rhs) const{  
    	return x * rhs.x + y * rhs.y;  
	}

	constexpr XY operator+(const XY& rhs)const{
		return {x + rhs.x, y + rhs.y};
	}

	constexpr XY& operator+=(const XY& rhs){
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	constexpr XY operator-()const{
		return {-x, -y};
	}

	constexpr XY operator-(const XY& rhs)const{
		return {x - rhs.x, y - rhs.y};
	}

	constexpr XY& operator-=(const XY& rhs){
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	constexpr XY operator*(T scalar)const{
		return {x * scalar, y * scalar};
	}

	constexpr XY& operator*=(T scalar){
		x *= scalar;
		y *= scalar;
		return *this;
	}

	constexpr XY operator/(T scalar) const{
		if(IsZero(scalar)){
			throw std::runtime_error("Division by zero in XY.");
		}
		return {x / scalar, y / scalar};
	}

	constexpr XY& operator/=(T scalar){
		return operator=(const_cast<const XY&>(*this) / scalar);
	}

	bool operator==(const XY& rhs)const{
		return IsZero(x - rhs.x) && IsZero(y - rhs.y);
	}

	bool operator!=(const XY& rhs)const{
		return !operator==(rhs);
	}	

	std::string toStr(char separator = ',')const requires detail::IsIntegral<T>{
		std::stringstream ss;
		ss << '(' << x << separator << y << ')';
		return ss.str();
	}

	std::string toStr(int precision = 3, char separator = ',')const requires detail::IsFloating<T>{
		std::stringstream ss;
		ss.setf(std::ios::fixed, std::ios::floatfield);
		ss.precision(precision);
		ss << '(' << x << separator << y << ')';
		return ss.str();
	}

	friend std::ostream& operator<<(std::ostream& os, const XY& xy){  
		os << "(" << xy.x << ", " << xy.y << ")";  
		return os;  
	} 

	constexpr XY()
		:x(T{}),
		y(T{}){
	}

	constexpr XY(T v_x, T v_y)
		:x(v_x),
		y(v_y){
	}

	constexpr XY(const T d[2])
		:x(d[0]),
		y(d[1]){
	}

	XY& operator=(const XY&) = default;
	constexpr XY(const XY&) = default;
	XY(XY&&) = default;
	~XY() = default;
};

//Some commonly used template classes are instantiated based on XY:
using Point2  = XY<double>;
using Pt2List = std::vector<Point2>;
using Point2i = XY<int>;
using Pt2iList = std::vector<Point2i>;
using Point2u = XY<unsigned int>;
using Pt2uList = std::vector<Point2u>;
using Vec2    = XY<double>;
using Vec2List = std::vector<Vec2>;

NAMESPACE_END

#endif