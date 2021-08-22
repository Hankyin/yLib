#pragma once
/**
 * @file netutils.h
 * @author yin
 * @brief 
 * @version 0.2
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
