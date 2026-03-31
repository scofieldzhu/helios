/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/3/1
*******************************************************/
#include "color.h"
#include <strstream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "str_util.h"

MIRFAK_NAMESPACE_BEGIN

const Color Color::Black("#000000");
const Color Color::White("#FFFFFF");
const Color Color::Red("#FF0000");
const Color Color::Green("#008000");
const Color Color::Blue("#0000FF");
const Color Color::Yellow("#FFFF00");
const Color Color::Purple("#800080");
const Color Color::Silver("#C0C0C0");
const Color Color::Gray("#808080");
const Color Color::Maroon("#800000");
const Color Color::Olive("#808000");
const Color Color::Lime("#00FF00");
const Color Color::Aqua("#00FFFF");
const Color Color::Teal("#008080");
const Color Color::Navy("#000080");
const Color Color::Fuchsia("#FF00FF");

Color::Color()
	:clr_({kMaxClrFieldInteger, kMaxClrFieldInteger, kMaxClrFieldInteger, kMaxClrFieldInteger})
{
}

Color::Color(int r, int g, int b, int a)
{
	setRed(r);
	setGreen(g);
	setBlue(b);
	setAlpha(a);
}

Color::Color(double r, double g, double b, double a)
{
	setRedR(r);
	setGreenR(g);
	setBlueR(b);
	setAlphaR(a);
}

Color::Color(const double* rgb)
{
    setRedR(rgb[0]);
	setGreenR(rgb[1]);
	setBlueR(rgb[2]);
    setAlphaR(1.0);
}

Color::Color(const std::string& hex_str)	
	:Color()
{
	setFromHexString(hex_str);
}

void Color::setRed(int r)
{
	clr_[0] = std::clamp(r, kMinClrFieldInteger, kMaxClrFieldInteger);
}

void Color::setRedR(double r)
{
	auto r_val = std::clamp(r, kMinClrFieldFloat, kMaxClrFieldFloat);
	clr_[0] = static_cast<int>(std::round(static_cast<double>(kMaxClrFieldInteger) * r_val));
}

double Color::redR() const
{
    return clr_[0] / static_cast<double>(kMaxClrFieldInteger);
}

void Color::setGreen(int g)
{
	clr_[1] = std::clamp(g, kMinClrFieldInteger, kMaxClrFieldInteger);
}

void Color::setGreenR(double g)
{
	auto g_val = std::clamp(g, kMinClrFieldFloat, kMaxClrFieldFloat);
	clr_[1] = static_cast<int>(std::round(static_cast<double>(kMaxClrFieldInteger) * g_val));
}

double Color::greenR() const
{
	return clr_[1] / static_cast<double>(kMaxClrFieldInteger);
}

void Color::setBlue(int b)
{
	clr_[2] = std::clamp(b, kMinClrFieldInteger, kMaxClrFieldInteger);
}

void Color::setBlueR(double b)
{
	auto b_val = std::clamp(b, kMinClrFieldFloat, kMaxClrFieldFloat);
	clr_[2] = static_cast<int>(std::round(static_cast<double>(kMaxClrFieldInteger) * b_val));
}

double Color::blueR() const
{
    return clr_[2] / static_cast<double>(kMaxClrFieldInteger);
}

void Color::setAlpha(int a)
{
	clr_[3] = std::clamp(a, kMinClrFieldInteger, kMaxClrFieldInteger);
}

void Color::setAlphaR(double a)
{
	auto a_val = std::clamp(a, kMinClrFieldFloat, kMaxClrFieldFloat);
	clr_[3] = static_cast<int>(std::round(static_cast<double>(kMaxClrFieldInteger) * a_val));	
}

double Color::alphaR() const
{
    return clr_[3] / static_cast<double>(kMaxClrFieldInteger);
}

void Color::setFromHexString(const std::string& hex_str)
{
	std::string pure_hex_str = TrimString(hex_str, {' ', '\n', '\r'});  
	if(pure_hex_str.empty()) {  
		throw std::invalid_argument("Empty hex string");  
	}  
	if(pure_hex_str[0] == '#') {  
		pure_hex_str.erase(0, 1);  
	}  
	if(pure_hex_str.size() != 6 && pure_hex_str.size() != 8) {  
		throw std::invalid_argument("Hex string should be 6 or 8 characters");  
	}  
	std::uint32_t value = std::stoul(pure_hex_str, nullptr, 16);  
	if(pure_hex_str.size() == 6) {  
		clr_[0] = static_cast<int>((value >> 16) & 0xFF);  
		clr_[1] = static_cast<int>((value >>  8) & 0xFF);  
		clr_[2] = static_cast<int>( value        & 0xFF);  
		clr_[3] = 255; 
	}else{  
		clr_[0] = static_cast<int>((value >> 24) & 0xFF);  
		clr_[1] = static_cast<int>((value >> 16) & 0xFF);  
		clr_[2] = static_cast<int>((value >>  8) & 0xFF);  
		clr_[3] = static_cast<int>( value        & 0xFF);  
	}
}

std::array<double, 4> Color::toValuesR() const
{
    return {redR(), greenR(), blueR(), alphaR()};
}

const int& Color::operator[](int index) const
{
	if(index < 0 || index >= 4){
		throw std::invalid_argument("Invalid subscript index!");
	}
    return clr_[index];
}

std::string Color::toHexString(bool with_alpha) const  
{  
	std::ostringstream oss;  
	oss << "#"  << std::hex 
		<< std::setw(2) << std::setfill('0') << static_cast<int>(clr_[0])  
		<< std::setw(2) << std::setfill('0') << static_cast<int>(clr_[1])  
		<< std::setw(2) << std::setfill('0') << static_cast<int>(clr_[2]);  
	if(with_alpha){  
		oss << std::setw(2) << std::setfill('0') << static_cast<int>(clr_[3]);  
	}  
	return oss.str();  
}

NAMESPACE_END