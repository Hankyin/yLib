#include "stringhelper.h"

std::vector<std::string> string_split(const std::string &_str, char ch)
{
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = _str.length();
    while (pos < len)
    {
        int find_pos = _str.find(ch, pos);
        if (find_pos < 0)
        {
            elems.push_back(_str.substr(pos, len - pos));
            break;
        }
        elems.push_back(_str.substr(pos, find_pos - pos));
        pos = find_pos + 1;
    }
    return elems;
}

std::string string_trim(const std::string &str)
{
    std::string st;
    int s = str.find_first_not_of(" \r\n\t");
    if (s == str.npos)
    {
        return st;
    }
    int e = str.find_last_not_of(" \r\n\t");
    st = str.substr(s, e - s + 1);
    return st;
}