#include "httpmessage.h"
#include "httpexcept.h"
#include "core/logger.h"
#include "core/stringhelper.h"

#include <string.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

namespace ylib
{

    std::string HTTPMethod_to_str(HTTPMethod m)
    {
        switch (m)
        {
        case HTTPMethod::GET:
            return "GET";
        case HTTPMethod::POST:
            return "POST";
        case HTTPMethod::HEAD:
            return "HEAD";
        case HTTPMethod::PUT:
            return "PUT";
        case HTTPMethod::DELETE:
            return "DELETE";
        case HTTPMethod::CONNECT:
            return "CONNECT";
        case HTTPMethod::OPTIONS:
            return "OPTIONS";
        case HTTPMethod::TRACE:
            return "TRACE";
        default:
            return "";
        }
    }

    std::string HTTPCode_to_str(int stat_code)
    {
        static std::map<int, std::string> code_map = {
            //消息响应
            {100, "Continue"},
            {101, "Switching Protocol"},
            // 成功相应
            {200, "OK"},
            {201, "Created"},
            {202, "Accepted"},
            {203, "Non-Authoritative Information"},
            {204, "No Content"},
            {205, "Reset Content"},
            {206, "Partial Content"},
            // 重定向
            {300, "Multiple Choice"},
            {301, "Moved Permanently"},
            {302, "Found"},
            {303, "Found"},
            {304, "Not Modified"},
            {305, "Use Proxy"},
            {306, "unused"},
            {307, "Temporary Redirect"},
            {308, "Permanent Redirect"},
            // 客户端错误
            {400, "Bad Request"},
            {401, "Unauthorized"},
            {402, "Payment Required"},
            {403, "Forbidden"},
            {404, "Not Found"},
            {405, "Method Not Allowed"},
            {406, "Not Acceptable"},
            {407, "Proxy Authentication Required"},
            {408, "Request Timeout"},
            {409, "Conflict"},
            {410, "Gone"},
            {411, "Length Required"},
            {412, "Precondition Failed"},
            {413, "Request Entity Too Large"},
            {414, "Request-URI Too Long"},
            {415, "Unsupported Media Type"},
            {416, "Requested Range Not Satisfiable"},
            {417, "Expectation Failed"},
            // 服务端错误
            {500, "Internal Server Error"},
            {501, "Implemented"},
            {502, "Bad Gateway"},
            {503, "Service Unavailable"},
            {504, "Gateway Timeout "},
            {505, "	HTTP Version Not Supported"},
        };
        //用[]会导致副作用.传入没有的会创建一个空字符串
        return code_map[stat_code];
    }

    HTTPMethod str_to_HTTPMethod(const std::string &str)
    {
        std::string st = string_trim(str);
        if (st == "GET")
        {
            return HTTPMethod::GET;
        }
        else if (st == "POST")
        {
            return HTTPMethod::POST;
        }
        else if (st == "HEAD")
        {
            return HTTPMethod::HEAD;
        }
        else if (st == "PUT")
        {
            return HTTPMethod::PUT;
        }
        else if (st == "DELETE")
        {
            return HTTPMethod::DELETE;
        }
        else if (st == "CONNECT")
        {
            return HTTPMethod::CONNECT;
        }
        else if (st == "OPTIONS")
        {
            return HTTPMethod::OPTIONS;
        }
        else if (st == "TRACE")
        {
            return HTTPMethod::TRACE;
        }
        else
        {
            return HTTPMethod::UNKNOW;
        }
    }

    void HTTPMsg::printf(const std::string &path) const
    {
        std::string http_content;
        http_content = first_line;
        http_content += "\r\n";
        for (auto &&kv : headers)
        {
            http_content += kv.first + ": " + kv.second + "\r\n";
        }
        http_content += "\r\n";
        http_content += body;

        if (path.empty())
        {
            std::cout << http_content << std::endl;
        }
        else
        {
            std::ofstream html_of(path, std::ios::trunc);
            if (html_of.is_open())
            {
                html_of << http_content;
            }
            else
            {
                LOG_ERROR << "http resp printf file fail";
            }
        }
    }

    //---------------------------- HTTP 消息解析部分-----------------------------//

    const std::string HTTPMsgParser::CRLF = "\r\n";

    HTTPMsgParser::HTTPMsgParser()
    {
    }

    HTTPMsgParser::~HTTPMsgParser()
    {
    }

    void HTTPMsgParser::parser_msg(HTTPMsg &msg, const std::string &buf, size_t start_pos = 0)
    {
        //读取第一行
        //HTTP/1.1 200 OK
        //POST /test/index.html HTTP/1.1
        // 每发现一行认为是一个header
        // 当发现一个空行时认为结束。
        size_t st = start_pos;
        size_t first_p = buf.find(CRLF, st);
        if(first_p == std::string::npos)
        {
            throw HTTPFormatException("can not find first line", buf.substr(0, 20));//异常只查看前20个字符。
        }
        msg.first_line = buf.substr(0, first_p);

        st = first_p + 2;
        while (true)
        {
            size_t p = buf.find(CRLF, st);
            if (p == st)
            {
                //找到了结尾，返回。
                return;
            }
            else if(p == std::string::npos)
            {
                //正常应该从上面的return结束。
                throw HTTPFormatException("finish abnormal ", buf.substr(0, 20));
            }
            std::string header_item = buf.substr(st, p - st);

            size_t colon_idx = header_item.find_first_of(':');

            if (colon_idx == header_item.npos)
            {
                throw HTTPFormatException("header format error", header_item);
            }
            std::string k = string_trim(header_item.substr(0, colon_idx));
            std::string v = string_trim(header_item.substr(colon_idx + 1));
            msg.headers[k] = v;

            st = p + 2;
        }
        return;
    }

    int HTTPMsgParser::parser_resp_line(HTTPVersion &version, int &code, std::string &code_line, const std::string &first_line)
    {

        auto first_vec = string_split(first_line, ' ');
        if (first_vec.size() != 3)
        {
            throw HTTPFormatException("http resp first line format error", first_line);
        }
        std::string v_str = string_trim(first_vec[0]);
        if (v_str == "HTTP/1.1")
        {
            version = HTTPVersion::V1_1;
        }
        else if (v_str == "HTTP/1.0")
        {
            version = HTTPVersion::V1_0;
        }
        else if (v_str == "HTTP/0.9")
        {
            version = HTTPVersion::V0_9;
        }
        else
        {
            throw HTTPFormatException("http version not support ", first_line);
        }

        code = ::atoi(first_vec[1].c_str());
        if (code == 0)
        {
            throw HTTPFormatException("http code is not a number", first_line);
        }

        code_line = first_vec[2];
        return 0;
    }

    int HTTPMsgParser::parser_req_line(HTTPVersion &version, HTTPMethod &method, std::string &path,
                                       std::map<std::string, std::string> &querys,
                                       const std::string &first_line)
    {
        auto first_vec = string_split(first_line, ' ');
        if (first_vec.size() != 3)
        {
            throw HTTPFormatException("http req first line format error", first_line);
        }
        method = str_to_HTTPMethod(first_vec[0]);

        std::string v_str = string_trim(first_vec[2]);
        if (v_str == "HTTP/1.1")
        {
            version = HTTPVersion::V1_1;
        }
        else if (v_str == "HTTP/1.0")
        {
            version = HTTPVersion::V1_0;
        }
        else if (v_str == "HTTP/0.9")
        {
            version = HTTPVersion::V0_9;
        }
        else
        {
            throw HTTPFormatException("http version not support ", first_line);
        }

        // 处理url路径
        // 例如:/study/video?class_id=24121&course_id=820&unit_id=13457#123
        auto fr_idx = first_vec[1].find_first_of('#');
        path = first_vec[1].substr(0, fr_idx); // 去掉# 没有的话fr_idx是npos，也无妨

        auto query_idx = path.find_first_of('?');
        if (query_idx != path.npos)
        {
            std::string query_str = path.substr(query_idx);
            auto query_map = string_split(query_str, '&');
            for (auto &kvq : query_map)
            {
                auto equal_idx = kvq.find_first_of('=');
                if (equal_idx != kvq.npos)
                {
                    std::string k = kvq.substr(0, equal_idx);
                    std::string v = kvq.substr(equal_idx);
                    querys[k] = v;
                }
            }
        }
        return 0;
    }

} // namespace ylib