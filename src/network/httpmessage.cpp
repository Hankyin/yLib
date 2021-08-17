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

    HTTPMsgParser::HTTPMsgParser(HTTPMsg &msg) : _msg(msg)
    {
    }

    HTTPMsgParser::~HTTPMsgParser()
    {
    }

    bool HTTPMsgParser::parser(const std::string &buf, size_t start_idx)
    {
        bool r = false;
        while (true)
        {
            switch (_parser_state)
            {
            case 0:
                r = read_firstline();
                if (r == false)
                {
                    return false;
                }
            case 1:
                r = read_header();
                if (r == false)
                {
                    return false;
                }
            case 2:
                // 头部已经读取完成
                return true;
            }
        }

        return false;
    }

    void HTTPMsgParser::reset()
    {
        _parser_state = 0;
        _header_start_pos = 0;
        _body_start_pos = 0;
    }

    bool HTTPMsgParser::read_firstline()
    {
        //读取第一行
        //HTTP/1.1 200 OK
        //POST /test/index.html HTTP/1.1
        auto p = _buf.find("\r\n");
        if (p == _buf.npos)
        {
            if (_buf.size() > _firstline_max_size) //首行太长了
            {
                throw HTTPTooLongException(0, _buf.size(), _firstline_max_size);
            }
            return false;
        }
        std::string head_line = _buf.substr(0, p);

        _msg.first_line = head_line;
        _header_start_pos = p + 2; //需要计算回车换行
        _parser_state = 1;
        return true;
    }

    bool HTTPMsgParser::read_header()
    {
        //如果只有首行，没有头部。
        if (_buf.substr(_header_start_pos, 2) == "\r\n")
        {
            _body_start_pos = _header_start_pos + 2;
            return true;
        }
        //有头部
        auto p = _buf.find("\r\n\r\n", _header_start_pos);
        if (p == _buf.npos)
        {
            if (_buf.size() > _header_start_pos) //header太长了
            {
                throw HTTPTooLongException(1, _buf.size(), _header_max_size);
            }
            return false;
        }
        std::string header = _buf.substr(_header_start_pos, p - _header_start_pos + 2); //算上最后的两个回车

        for (size_t i = 0; i < header.size();)
        {
            size_t hp = header.find("\r\n", i);
            std::string header_item = header.substr(i, hp - i);
            size_t colon_idx = header_item.find_first_of(':');

            if (colon_idx == header_item.npos)
            {
                throw HTTPFormatException("header format error", header_item);
            }
            std::string k = string_trim(header_item.substr(0, colon_idx));
            std::string v = string_trim(header_item.substr(colon_idx + 1));
            _msg.headers[k] = v;
            i = hp + 2;
        }
        _body_start_pos = p + 4;
        _parser_state = 2;
        return true;
    }



    HTTPReqHeaderParser::HTTPReqHeaderParser(HTTPRequestMsg &req)
        : HTTPHeaderParser(req), _req(req)
    {
    }

    HTTPReqHeaderParser::~HTTPReqHeaderParser()
    {
    }

    bool HTTPReqHeaderParser::parser()
    {
        if (HTTPHeaderParser::parser() == false)
        {
            return false;
        }
        //父类已经处理完成。
        std::string first_line = _req.first_line;
        auto first_vec = string_split(first_line, ' ');
        if (first_vec.size() != 3)
        {
            throw HTTPFormatException("http req first line format error", first_line);
        }
        _req.method = str_to_HTTPMethod(first_vec[0]);
        _req.version = first_vec[2];
        std::string total_path = first_vec[1]; // 带参数的path
        // 解析url中query部分,假设没有fragment部分
        // TODO将这部分整合到URL类中
        auto query_idx = total_path.find_first_of('?');
        if (query_idx != total_path.npos)
        {
            std::string query_str = total_path.substr(query_idx);
            auto query_map = string_split(query_str, '&');
            for (auto &kvq : query_map)
            {
                auto equal_idx = kvq.find_first_of('=');
                if (equal_idx != kvq.npos)
                {
                    std::string k = kvq.substr(0, equal_idx);
                    std::string v = kvq.substr(equal_idx);
                    _req.querys[k] = v;
                }
            }
        }
        _req.path = first_vec[1];

        return true;
    }

    HTTPRespHeaderParser::HTTPRespHeaderParser(HTTPResponseMsg &resp)
        : HTTPHeaderParser(resp), _resp(resp)
    {
    }

    HTTPRespHeaderParser::~HTTPRespHeaderParser()
    {
    }

    bool HTTPRespHeaderParser::parser()
    {
        if (HTTPHeaderParser::parser() == false)
        {
            return false;
        }
        //父类已经处理完成。
        std::string first_line = _resp.first_line;
        auto first_vec = string_split(first_line, ' ');
        if (first_vec.size() != 3)
        {
            throw HTTPFormatException("http resp first line format error", first_line);
        }
        _resp.version = first_vec[0];
        _resp.code = ::atoi(first_vec[1].c_str());
        _resp.code_line = first_vec[2];
        return true;
    }

} // namespace ylib