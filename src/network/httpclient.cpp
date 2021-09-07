#include "httpclient.h"
#include "netutils.h"
#include "httpmessage.h"
#include "httpexcept.h"

#include "core/stringhelper.h"
#include "core/logger.h"

#include <iostream>
#include <algorithm>

namespace ylib
{

    HTTPResponseMsg HTTPClient::GET(const std::string &url_str)
    {
        std::string http_ip;
        URL _url(url_str);
        if (is_ipv4(_url.host))
        {
            http_ip = _url.host;
        }
        else
        {
            http_ip = resolve_hostname(_url.host);
        }
        HTTPClient cli;
        HTTPRequestMsg req;
        HTTPResponseMsg resp;
        req.method = HTTPMethod::GET;
        req.path = _url.path;
        cli.connect(http_ip, _url.port);
        cli.send_req(req);
        cli.recv_resp(resp);
        return resp;
    }

    HTTPResponseMsg HTTPClient::POST(const std::string &url, const std::string &body)
    {
    }

    HTTPClient::HTTPClient()
    {
    }

    HTTPClient::~HTTPClient()
    {
        close();
    }

    void HTTPClient::connect(const std::string &ip, uint16_t port)
    {
        _socket.connect(ip, port);
        LOG_TRACE << "http client connect to " << ip << " : " << port;
        _stat = 1;
    }

    void HTTPClient::close()
    {
        // socket的close可以多次调用没关系
        _socket.close();
        _stat = 0;
    }

    void HTTPClient::send_req(const HTTPRequestMsg &req)
    {
        LOG_TRACE << "http client send request start";
        // GET /test/index.html?id=1&name=abc
        std::string head_line;
        std::string header_part;

        std::string full_path = req.path;
        if (full_path.empty())
            full_path = "/";
        if (!req.querys.empty())
        {
            full_path.push_back('?');
            for (auto &i : req.querys)
            {
                std::string p = i.first + "=" + i.second + "&";
                full_path += p;
            }
            full_path.pop_back();
        }

        head_line = HTTPMethod_to_str(req.method);
        head_line += ' ';
        head_line += full_path;
        head_line += " HTTP/1.1";
        head_line += HTTPMsgParser::CRLF;

        for (auto &&kv : req.headers)
        {
            std::string h;
            h = kv.first;
            h += ": ";
            h += kv.second;
            h += HTTPMsgParser::CRLF;
            header_part += h;
        }
        header_part += HTTPMsgParser::CRLF;

        _socket.write_str(head_line);
        _socket.write_str(header_part);
        _socket.write_str(req.body); //TODO 增加大文件上传功能
        LOG_TRACE << "http client send request finish";
    }

    void HTTPClient::recv_resp(HTTPResponseMsg &resp)
    {
        LOG_TRACE << "http client receive response start";
        int pknum = 0;
        size_t st = 0;
        size_t body_boundary = 0; // header和body的分界线
        size_t next_req_idx = 0;  // 下一个http 请求的分界线。
        HTTPMsgParser msg_parser;
        std::string double_CRLF = HTTPMsgParser::CRLF + HTTPMsgParser::CRLF;

        // 头部信息读取
        LOG_TRACE << "http client read resp head";
        while (true)
        {
            //首先判断头部消息是否完整，根据两个回车换行判断。
            //头部消息完整后再调用http解析器处理。
            // httpbuf可能为0，也可能有上次残留的数据。
            size_t p = http_buf.find(double_CRLF, st);
            if (p == std::string::npos)
            {
                if (pknum > _header_pack_num)
                {
                    throw HTTPTooLongException(_header_pack_num, _pack_size); //过长报错
                }
                else
                {
                    // 更新st值，避免重复查找
                    // 每次往回倒三个字符，避免当前缓存最后是/r/n/r的情况。
                    st = http_buf.size() > 3 ? http_buf.size() - 3 : 0; // 避免httpbuf的size小于0

                    std::string pack = _socket.read(_pack_size); // 从socket中读取数据到缓冲中
                    http_buf += pack;
                    pknum++;
                    LOG_TRACE << pack;
                    continue;
                }
            }
            else
            {
                body_boundary = p + 4;
                break;
            }
        }

        //头部已经被解析完成，根据header的信息进行读取body等处理。
        // 目前支持Content-Length，暂不支持gzip，chunk等模式
        LOG_TRACE << "http client parser resp head";
        std::string header_part = http_buf.substr(0, body_boundary);
        next_req_idx = body_boundary;
        msg_parser.parser_msg(resp, header_part);
        msg_parser.parser_resp_line(resp.version, resp.code, resp.code_line, resp.first_line);

        //Content-Length处理，对body进行处理
        LOG_TRACE << "http client read body";
        if (resp.headers.find(HTTPMsg::ContentLength) != resp.headers.end())
        {
            LOG_TRACE << "http client find Content-Length ";
            size_t body_len = getContentLength(resp.headers);
            LOG_TRACE << "body length is " << body_len << " byte";
            if (resp.callback == nullptr)
            {
                //默认处理
                // 此时httpbuf中的数据可能有如下几种情况
                // 1. 没有剩余数据了。
                // 2. 有一部分当前msg的body数据
                // 3. 有当前msg的全部body，还有下一个msg的数据。
                LOG_TRACE << "read body by default";
                resp.body = http_buf.substr(body_boundary, body_len);
                next_req_idx = body_boundary + resp.body.size();
                size_t left_len = body_len - resp.body.size();
                resp.body += _socket.readn(left_len);
            }
            else
            {
                //调用客户自己写的回调。
                LOG_TRACE << "read body by callback";
                int stat = 1; // 1 代表还有数据，0 代表数据传输结束
                size_t left_len = body_len;
                std::string body_buf = http_buf.substr(body_boundary, body_len);
                next_req_idx = body_boundary + body_buf.size();
                left_len -= body_buf.size();
                stat = left_len == 0 ? 0 : 1;
                resp.callback(resp, body_buf, stat);

                size_t pre_pack;
                while (left_len)
                {
                    pre_pack = std::min(_pack_size, left_len);
                    body_buf = _socket.read(pre_pack);
                    left_len -= body_buf.size();
                    stat = left_len == 0 ? 0 : 1;
                    resp.callback(resp, body_buf, stat);
                }
            }
        }
        else
        {
            LOG_TRACE << "current resp don't have body";
        }

        // 一个http回复已经完成，处理http_buf
        http_buf = http_buf.substr(next_req_idx);

        LOG_TRACE << "http client receive response finish";
    }

    size_t HTTPClient::getContentLength(std::map<std::string, std::string> headers)
    {
        try
        {
            size_t body_len = 0;
            body_len = std::stoul(headers.at(HTTPMsg::ContentLength));
            return body_len;
        }
        catch (const std::exception &e)
        {
            // 产生异常，认为解析失败。
            // 比如没有contentlength字段
            // 比如长度无法被解析。
            return 0;
        }
    }

} // namespace ylib
