#pragma once

#include <stdexcept>

#include <string>

namespace ylib
{

    //http的异常类
    class HTTPException : public std::runtime_error
    {
    public:
        HTTPException(const std::string &info)
            : std::runtime_error(info)
        {
        }
    };
    //http过长异常类
    class HTTPTooLongException : public HTTPException
    {
    public:
        HTTPTooLongException(int pt, int c_size, int mx_size)
            : HTTPException(std::string("http too long exception [") + (pt == 0 ? "firstline:" : "header:") + std::to_string(cur_size) + '/' + std::to_string(max_size) + "]")
        {
            part = pt;
            cur_size = c_size;
            max_size = mx_size;
        }
        int part; // 0 首行过长， 1 头部过长
        size_t cur_size;
        size_t max_size;
    };

    // http 语法错误异常
    class HTTPFormatException : public HTTPException
    {
    public:
        HTTPFormatException(const std::string &desc, const std::string &ctx)
            : HTTPException(desc + "[" + ctx + "]")
        {
        }
    };

} // namespace ylib
