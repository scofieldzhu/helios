/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2025-2025)
* author: zhucg
* time:2025/3/1
*******************************************************/
#include "str_util.h"
#include <algorithm>

MIRFAK_NAMESPACE_BEGIN

std::string TrimString(const std::string& s, std::initializer_list<unsigned char> dirty_chars)  
{  
    if(std::empty(dirty_chars)){
        return s;
    }
    auto check_dirty_func = [&dirty_chars](unsigned char c){
        return std::any_of(dirty_chars.begin(), 
                           dirty_chars.end(), 
                           [c](unsigned char d){
                               return d == c;
                           });
    }; 
    auto start = std::find_if_not(s.begin(), s.end(), check_dirty_func);  
    auto end = std::find_if_not(s.rbegin(), s.rend(), check_dirty_func).base();  
    return start < end ? std::string(start, end) : std::string{};  
}  

NAMESPACE_END