#pragma once

//实现了一些string的常用操作。

#include <string>
#include <vector>

std::vector<std::string> string_split(const std::string &_str, char ch); //根据字符分割字符串
std::string string_trim(const std::string &str);                         //去除字符串前后空白，回车，换行等