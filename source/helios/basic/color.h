/******************************************************** 
* author: scofieldzhu
* time:2025/3/1
*******************************************************/
#ifndef __color_h__
#define __color_h__

#include <array>
#include <limits>
#include <string>
#include <optional>
#include "helios/basic/helios_basic_export.h"

HELIOS_NAMESPACE_BEGIN

class HELIOS_BASIC_API Color
{
public:
	static constexpr int kMinClrFieldInteger = std::numeric_limits<std::uint8_t>::min();
	static constexpr int kMaxClrFieldInteger = std::numeric_limits<std::uint8_t>::max();
	static constexpr double kMinClrFieldFloat = 0.0;
	static constexpr double kMaxClrFieldFloat = 1.0;
	static const Color Black;
	static const Color White;
	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Yellow;
	static const Color Purple;
	static const Color Silver;
	static const Color Gray;
	static const Color Maroon;
	static const Color Olive;
	static const Color Lime;
	static const Color Aqua;
	static const Color Teal;
	static const Color Navy;
	static const Color Fuchsia;
	void setRed(int r);
	void setRedR(double r);
	int red()const { return clr_[0]; }
	double redR()const;
	void setGreen(int g);
	void setGreenR(double g);
	int green()const { return clr_[1]; }
	double greenR()const;
	void setBlue(int b);
	void setBlueR(double b);
	int blue()const { return clr_[2]; }
	double blueR()const;
	void setAlpha(int a);
	void setAlphaR(double a);
	int alpha()const { return clr_[3]; }
	double alphaR()const;
	void setFromHexString(const std::string& hex_str);
	std::array<double, 4> toValuesR()const;
	const std::array<int, 4>& toValues()const{ return clr_; }
	const int& operator[](int index)const;
	std::string toHexString(bool with_alpha = false) const;
	Color();
	Color(int r, int g, int b, int a = kMaxClrFieldInteger);
	Color(double r, double g, double b, double a = kMaxClrFieldFloat);
    Color(const double* rgb);
	Color(const std::string& hex_str);

private:
	std::array<int, 4> clr_;	
};

using ColorOpt = std::optional<Color>;

NAMESPACE_END

#endif
