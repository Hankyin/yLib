#include "network.h"

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <string.h>

#include <regex>

namespace ylib
{

    URL::URL(const std::string &http_url)
    {
        parse_url(http_url);
    }

    int URL::parse_url(const std::string &http_url)
    {
        //协议://用户名:密码@子域名.域名.顶级域名:端口号/目录/文件名.文件后缀?参数=值#标志
        try
        {
            url = http_url;
            is_legal = false;
            //协议判断
            std::size_t scheme_idx = url.find(':');
            scheme = url.substr(0, scheme_idx);

            if (scheme == "http")
            {
                port = 80;
            }
            else if (scheme == "https")
            {
                port = 443;
            }
            else
            {
                return -1;
            }

            // ://
            if (url.substr(scheme_idx, 3) != "://")
            {
                return -1;
            }
            std::size_t host_idx = scheme_idx + 3;
            // 可选用户名和密码

            std::size_t at_idx = url.find('@', scheme_idx + 3);
            if (at_idx != url.npos)
            {
                std::string userinfo = url.substr(scheme_idx + 3, at_idx - (scheme_idx + 3));
                std::size_t p_idx = userinfo.find(':');
                user = userinfo.substr(0, p_idx);
                passwd = userinfo.substr(p_idx + 1);
                host_idx += userinfo.size() + 1; //要算上@
            }

            // host 和 port
            std::string host_and_port;
            std::size_t path_idx = url.find('/', host_idx);
            if (path_idx == url.npos)
            {
                host_and_port = url.substr(host_idx);
                url.push_back('/');
            }
            else
            {
                host_and_port = url.substr(host_idx, path_idx - host_idx);
            }

            std::size_t port_idx = host_and_port.find(':');
            if (port_idx != host_and_port.npos)
            {
                host = host_and_port.substr(0, port_idx);
                port = std::stoi(host_and_port.substr(port_idx + 1)); //去掉:
            }
            else
            {
                host = host_and_port;
            }
            if (path_idx == url.npos) //url已经结束，没有后面的路径
            {
                is_legal = true;
                path = "/";
                pure_path = "/";
                return 0;
            }

            // path 去掉#就是path
            std::size_t fragment_idx = url.find('#', path_idx);
            if (fragment_idx != url.npos)
            {
                path = url.substr(path_idx, fragment_idx - path_idx);
            }
            else
            {
                path = url.substr(path_idx);
            }

            //pure_path path去掉？就是纯path

            std::size_t query_idx = url.find('?', path_idx);

            if (query_idx != url.npos) //query和fragment都是可选的，同时query在前面
            {
                pure_path = url.substr(path_idx, query_idx - path_idx);
            }
            else
            {
                pure_path = path;
                is_legal = true;
            }

            //query fragment 可选
            if (query_idx != url.npos && fragment_idx != url.npos)
            {
                query = url.substr(query_idx + 1, fragment_idx - query_idx - 1);
                fragment = url.substr(fragment_idx + 1);
            }
            else if (fragment_idx == url.npos)
            {
                query = url.substr(query_idx + 1);
            }
            else
            {
                fragment = url.substr(fragment_idx + 1);
            }
            is_legal = true;
        }
        catch (const std::exception &e)
        {
            // std::cerr << e.what() << '\n';
            is_legal = false;
            return -1;
        }
        return 0;
    }

    std::string &URL::assembly_url()
    {
        //协议://用户名:密码@子域名.域名.顶级域名:端口号/目录/文件名.文件后缀?参数=值#标志
        if (url.empty())
        {
            url = scheme + "://";
            if (!user.empty())
            {
                url += user + ":" + passwd + "@";
            }
            url += host + ":" + std::to_string(port) + path;
        }
        return url;
    }

    static __thread char t_resolveBuffer[64 * 1024];

    std::string resolve_hostname(const std::string &hostname)
    {
        std::string add_str;
        if (false)
        {
            //这个
            struct addrinfo hints;
            struct addrinfo *dns_result, *rp;
            ::memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            int info_r = ::getaddrinfo(hostname.c_str(), NULL, &hints, &dns_result);
            if (info_r != 0)
            {
                throw DNSResolveException(::gai_strerror(info_r), info_r);
            }
            for (rp = dns_result; rp != NULL; rp = rp->ai_next)
            {
                char ip_buf[INET_ADDRSTRLEN] = {0};
                ::inet_ntop(AF_INET, rp->ai_addr, ip_buf, INET_ADDRSTRLEN);
                if (::strlen(ip_buf) != 0)
                {
                    add_str = ip_buf;
                }
            }
            freeaddrinfo(dns_result);
        }
        else
        {
            struct hostent hent;
            struct hostent *he = NULL;
            int herrno = 0;
            ::memset(&hent, 0, sizeof(hent));

            int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
            if (ret == 0 && he != NULL)
            {
                char ip_buf[INET_ADDRSTRLEN] = {0};
                ::inet_ntop(AF_INET, he->h_addr, ip_buf, INET_ADDRSTRLEN);
                if (::strlen(ip_buf) != 0)
                {
                    add_str = ip_buf;
                }
            }
            else
            {
                throw DNSResolveException(::hstrerror(herrno), herrno);
            }
        }
        return std::move(add_str);
    }

    bool is_ipv4(const std::string &ip)
    {
        int no = 3;
        switch (no)
        {
        case 1:
            //方法1 手动分析字符串
            {
            }
            break;
        case 2:
            //方法2 正则表达式
            {
                std::regex ipreg(R"(^((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3}$)");
                return std::regex_match(ip, ipreg);
            }
            break;
        case 3:
            //方法3 使用inet_pton函数
            //测试后发现这个方式最快
            {
                in_addr addr;
                if (::inet_pton(AF_INET, ip.c_str(), &addr) != 1)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            break;
        default:
            break;
        }
        return false;
    }
} // namespace ylib
