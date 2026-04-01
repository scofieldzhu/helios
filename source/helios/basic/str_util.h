/******************************************************** 
* author: scofieldzhu
* time:2025/3/1
*******************************************************/
#ifndef __str_util_h__
#define __str_util_h__

#include "helios/basic/helios_basic_export.h"
#include <string>
#include <initializer_list>

HELIOS_NAMESPACE_BEGIN

HELIOS_BASIC_API std::string TrimString(const std::string& s, std::initializer_list<unsigned char> dirty_chars);

NAMESPACE_END

#endif