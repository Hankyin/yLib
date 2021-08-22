#pragma once

#include <stdexcept>

#include <string>

namespace ylib
{
    //socket的异常类
    class SocketException : public std::runtime_error
    {
    public:
        SocketException(const std::string &info, int err_no)
            : std::runtime_error(info)
        {
            error_no = err_no;
        }
        int error_no = 0;
    };
    //socket 创建异常
    class SocketCreateException : public SocketException
    {
    public:
        SocketCreateException(const std::string &info, int err_no)
            : SocketException("socket create exception [" + info + "]", err_no)
        {
        }
    };
    //socket 对端关闭
    class SocketCloseException : public SocketException
    {
    public:
        SocketCloseException(const std::string &info, int err_no)
            : SocketException("peer socket close [" + info + "]", err_no)
        {
        }
        //对端关闭时已经发送/读取的数据长度
        int process_data_len;
    };
    //socket读取异常
    class SocketReadException : public SocketException
    {
    public:
        SocketReadException(const std::string &info, int err_no)
            : SocketException("socket read exception [" + info + "]", err_no)
        {
        }
    };

    //socket发送异常
    class SocketWriteException : public SocketException
    {
    public:
        SocketWriteException(const std::string &info, int err_no)
            : SocketException("socket write exception [" + info + "]", err_no)
        {
        }
    };

    //socket 读写时被信号打断异常
    class SocketINTRException : public SocketException
    {
    public:
        SocketINTRException(const std::string &info, int err_no)
            : SocketException("socket interrupt exception [" + info + "]", err_no)
        {
        }
    };

    //IP转换异常
    class SocketIPException : public SocketException
    {
    public:
        SocketIPException(const std::string &info, int err_no)
            : SocketException("IP abnormal [" + info + "]", err_no)
        {
        }
    };

    //设置
    class SocketOptException : public SocketException
    {
    public:
        SocketOptException(const std::string &info, int err_no)
            : SocketException("socket opt exception [" + info + "]", err_no)
        {
        }
    };

    //连接服务器异常
    class SocketConnectException : public SocketException
    {
    public:
        SocketConnectException(const std::string &info, int err_no)
            : SocketException("socket connect exception [" + info + "]", err_no)
        {
        }
    };

    // socket 绑定 ip 异常
    class SocketBindException : public SocketException
    {
    public:
        SocketBindException(const std::string &info, int err_no)
            : SocketException("socket bind exception [" + info + "]", err_no)
        {
        }
    };

    // socket 监听 异常
    class SocketListenException : public SocketException
    {
    public:
        SocketListenException(const std::string &info, int err_no)
            : SocketException("socket listen exception [" + info + "]", err_no)
        {
        }
    };

    // socket accept 客户端异常
    class SocketAcceptException : public SocketException
    {
    public:
        SocketAcceptException(const std::string &info, int err_no)
            : SocketException("socket accept exception [" + info + "]", err_no)
        {
        }
    };

} // namespace ylib
