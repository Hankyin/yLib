#pragma once
/**
 * @file network.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-05-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <list>
#include <stdexcept>
#include <string>

#include <netinet/in.h>

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

    //dns域名解析异常类
    class DNSResolveException : public std::runtime_error
    {
    public:
        DNSResolveException(const std::string &info, int err_no)
            : std::runtime_error(info)
        {
            error_no = err_no;
        }
        int error_no = 0;
    };

    /**
     * @brief URL处理类
     * @todo URL解析出错抛出异常
     * 
     */
    struct URL
    {
        URL() {}
        URL(const std::string &http_url);
        int parse_url(const std::string &http_url);
        std::string &assembly_url();
        std::string url;
        std::string scheme;
        std::string user;
        std::string passwd;
        std::string host;
        int port;
        std::string path; //带query的path
        std::string pure_path;
        std::string query;
        std::string fragment; //fragment是用来提示浏览器的，服务器不需要这个。
        bool is_legal = false;
    };

    /**
    * @brief 调用Linux系统函数进行dns解析
    * 
    * @param hostname 
    * @return std::string
    * @exception DNSResolveException  DNS系统调用出错
    */
    std::string resolve_hostname(const std::string &hostname);

    /**
     * @brief 检测传入字符串是否为IPv4字符串
     * 
     * @param ip 
     * @return true 
     * @return false 
     */
    bool is_ipv4(const std::string &ip);

} // namespace ylib
