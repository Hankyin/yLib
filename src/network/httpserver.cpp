#include "httpserver.h"
#include "httpexcept.h"
#include "core/logger.h"
#include "core/stringhelper.h"

#include <iostream>
#include <thread>
#include <algorithm>

namespace ylib
{
    HTTPServer::HTTPServer() : _server_status(0)
    {
    }

    HTTPServer::~HTTPServer()
    {
    }

    void HTTPServer::add_handler(const std::string &url_pattern, HTTPMethod method,
                                 HTTP_handler_func handler, HTTPMsg::HTTP_msg_callback req_callback)
    {
        Handler h = {url_pattern, handler, req_callback};
        switch (method)
        {
        case HTTPMethod::GET:
            _GET_handlers.push_back(h);
            break;
        case HTTPMethod::POST:
            _POST_handlers.push_back(h);
            break;
        default:
            break;
        }
    }

    void HTTPServer::run(const std::string &ip, uint16_t port, int backlog)
    {
        _server.setopt_REUSEADDR();
        _server.bind(ip, port);
        _server.listen(backlog);
        _server_status = 1;
        LOG_INFO << "HTTP server start run, IP:" << ip << ", port:" << port;

        while (_server_status == 1)
        {
            TCPSocket sock = _server.accept();
            LOG_DEBUG << "client connect, ip: " << sock.peerIP() << " port:" << sock.peerPort();
            // TODO
            //根据配置选择不同的处理模式
            // 每个请求一个进程，或者使用进程池
            if (true)
            {
                // 每个请求一个线程
                std::thread th(&HTTPServer::process, this, std::move(sock));
                th.detach();
            }
            else
            {
                //线程池处理 TODO
            }
        }
        return;
    }

    void HTTPServer::process(TCPSocket sock)
    {
        LOG_TRACE << "http server conn processing start ";
        try
        {
            std::string http_buf; // http缓冲
            while (true)          //一次连接可能进行多次交互。
            {
                HTTPRequestMsg req;
                HTTPResponseMsg resp;

                size_t body_boundary = sock_recv_req_head(req, sock, http_buf);

                //路由解析，获取handler
                LOG_TRACE << "http router url " << string_trim(req.path);
                Handler cur_handler = url_router(req);
                req.callback = cur_handler.req_callback;
                //读取body部分
                size_t next_req_idx = sock_recv_req_body(req, sock, http_buf, body_boundary);

                //调用HTTT url 的handler
                LOG_TRACE << "exec url handler";
                std::vector<std::string> arg; //todo
                cur_handler.handler(req, resp, arg);

                //handler构造了resp，将resp发送出去。
                sock_send_resp(resp, sock);

                //删掉缓冲中本次已经读取的数据。
                http_buf = http_buf.substr(next_req_idx);
                LOG_TRACE << "http request finish ";
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR << e.what();
        }
        LOG_DEBUG << "http server conn process finish";
    }

    HTTPServer::Handler &HTTPServer::url_router(const HTTPRequestMsg &req)
    {
        std::vector<Handler>::iterator h;
        switch (req.method)
        {
        case HTTPMethod::GET:
            for (auto &&h : _GET_handlers)
            {
                if (h.url_pattern == req.path)
                    return h;
            }
            break;
        case HTTPMethod::POST:
            for (auto &&h : _POST_handlers)
            {
                if (h.url_pattern == req.path)
                    return h;
            }
            break;
        default:
            break;
        }
        //运行完switch还没有返回，就认为无法路由。
        throw HTTPRouteException(req.path, HTTPMethod_to_str(req.method));
    }

    size_t HTTPServer::sock_recv_req_head(HTTPRequestMsg &req, TCPSocket &sock, std::string &http_buf)
    {
        int pknum = 0;
        size_t st = 0;
        size_t body_boundary = 0; // header和body的分界线
        HTTPMsgParser msg_parser;
        std::string double_CRLF = HTTPMsgParser::CRLF + HTTPMsgParser::CRLF;
        // 头部信息读取
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
                    //更新st值，避免重复查找
                    // 每次往回倒三个字符，避免当前缓存最后是/r/n/r的情况。
                    st = http_buf.size() > 3 ? http_buf.size() - 3 : 0; // 避免httpbuf的size小于0

                    std::string pack = sock.read(_pack_size); // 从socket中读取数据到缓冲中
                    http_buf += pack;
                    pknum++;
                    continue;
                }
            }
            else
            {
                body_boundary = p + 4;
                break;
            }
        }
        // 解析请求的头部
        LOG_TRACE << "http parser request message header";
        std::string header_part = http_buf.substr(0, body_boundary);
        msg_parser.parser_msg(req, header_part);
        msg_parser.parser_req_line(req.version, req.method, req.path, req.querys, req.first_line);

        return body_boundary;
    }

    size_t HTTPServer::sock_recv_req_body(HTTPRequestMsg &req, TCPSocket &sock, std::string &http_buf, size_t body_boundary)
    {
        size_t next_req_idx = body_boundary;
        // 基于contentlength处理
        if (req.headers.find(HTTPMsg::ContentLength) != req.headers.end())
        {
            size_t body_len = std::stoul(req.headers.at(HTTPMsg::ContentLength));
            LOG_TRACE << "HTTP request Content-Length:" << body_len;

            if (req.callback == nullptr)
            {
                //默认处理
                LOG_TRACE << "read request body";
                req.body = http_buf.substr(body_boundary, body_len);
                next_req_idx = body_boundary + req.body.size();
                size_t left_len = body_len - req.body.size();
                req.body += sock.readn(left_len);
            }
            else
            {
                //调用客户自己写的回调。
                LOG_TRACE << "read request body by callback";
                int stat = 1; // 1 代表还有数据，0 代表数据传输结束
                size_t left_len = body_len;
                std::string body_buf = http_buf.substr(body_boundary, body_len);
                next_req_idx = body_boundary + body_buf.size();
                left_len -= body_buf.size();
                stat = left_len == 0 ? 0 : 1;
                req.callback(req, body_buf, stat);

                size_t pre_pack;
                while (left_len)
                {
                    pre_pack = std::min(_pack_size, left_len);
                    body_buf = sock.read(pre_pack);
                    left_len -= body_buf.size();
                    stat = left_len == 0 ? 0 : 1;
                    req.callback(req, body_buf, stat);
                }
            }
        }

        return next_req_idx;
    }

    void HTTPServer::sock_send_resp(HTTPResponseMsg &resp, TCPSocket &sock)
    {
        if (resp.code_line.empty())
        {
            resp.code_line = HTTPCode_to_str(resp.code);
        }
        resp.first_line = HTTPVersion_to_str(resp.version) +
                          " " + std::to_string(resp.code) +
                          " " + resp.code_line;
        resp.first_line += HTTPMsgParser::CRLF;

        LOG_TRACE << "http send resp " << string_trim(resp.first_line);

        std::string header_part;
        // 自动产生一些头部信息
        resp.headers["Server"] = _server_name;
        if (resp.callback == nullptr)
            resp.headers[resp.ContentLength] = std::to_string(resp.body.size());

        for (auto &kv : resp.headers)
        {
            std::string hl = kv.first + ": " + kv.second;
            header_part += hl;
            header_part += HTTPMsgParser::CRLF;
        }
        header_part += HTTPMsgParser::CRLF;
        sock.write_str(resp.first_line);
        sock.write_str(header_part);
        sock.write_str(resp.body);
    }

    void HTTPServer::sock_send_404(TCPSocket &sock)
    {
    }

    void HTTPServer::sock_send_500(TCPSocket &sock)
    {
    }

} // namespace ylib
