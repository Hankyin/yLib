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
        HTTPTooLongException(size_t packsize, uint32_t packnum)
            : HTTPException(std::string("http header too long [") + std::to_string(packsize) + '/' + std::to_string(packnum) + "]")
        {
        }
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
