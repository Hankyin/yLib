#include "httpserver.h"
#include "httpexcept.h"
#include "core/logger.h"

#include <iostream>
#include <thread>

namespace ylib
{
    HTTPServer::HTTPServer()
    {
    }

    HTTPServer::~HTTPServer()
    {
    }
    int HTTPServer::run(const std::string &ip, uint16_t port, int backlog)
    {
        _server.setopt_REUSEADDR();
        _server.bind(ip, port);
        _server.listen(backlog);
        _server_status = 1;
        LOG_INFO << "HTTP server start run, IP:" << ip << ", port:" << port;

        while (_server_status == 1)
        {
            TCPSocket sock = _server.accept();
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

        _server_status = 0;
        return 0;
    }

    void HTTPServer::process(TCPSocket sock)
    {
        LOG_DEBUG << "http server processing";
        try
        {
            while (true)
            {
                HTTPRequestMsg req;
                HTTPResponseMsg resp;

                LOG_TRACE << "http get request " << req.first_line;
                HTTPReqHeaderParser rq(req);
                while (true)
                {
                    std::string recv = sock.read(4096);
                    rq.buffer_add(recv);
                    if (rq.parser())
                    {
                        req.body = rq.get_left_str();
                        break;
                    }
                }

                LOG_TRACE << "http make response " << req.first_line;
                //url路由
                std::vector<std::string> arg; //TODO url 中的捕获的参数，restful接口要用,还没实现
                //创建body读取对象
                HTTPServerBodyReader body_reader(sock, req);
                // 根据URL匹配对应的handler，并且执行
                exec_handler(req, resp, body_reader, arg);
                // 对response进行构造
                if (resp.first_line.empty())
                {
                    if (resp.version.empty())
                    {
                        resp.version = "HTTP/1.1";
                    }
                    resp.first_line = resp.version + ' ' + std::to_string(resp.code) + HTTPCode_to_str(resp.code);
                }
                resp.headers["Content-Length"] = std::to_string(resp.body.size());
                resp.headers["Server"] = _server_name;
                sock_send_resp(resp, sock);
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR << e.what();
        }
        LOG_DEBUG << "http server process finish";
    }

    void HTTPServer::exec_handler(HTTPRequestMsg &req, HTTPResponseMsg &resp,
                                  HTTPServerBodyReader &body, std::vector<std::string> arg)
    {
        switch (req.method)
        {
        case HTTPMethod::GET:
            for (auto h : _GET_handlers)
            {
                if (h.url_pattern == req.path)
                {
                    h.handler(req, resp, body, arg);
                }
            }
            break;

        default:
            break;
        }
    }

    void HTTPServer::sock_send_resp(HTTPResponseMsg &resp, TCPSocket &sock)
    {
        LOG_TRACE << "http send resp" << resp.first_line;
        std::string firstline_part;
        std::string header_part;
        firstline_part = resp.version + " " + std::to_string(resp.code) + " " + resp.code_line;
        for (auto &kv : resp.headers)
        {
            std::string hl = kv.first + ": " + kv.second;
            header_part += hl;
            header_part += "\r\n";
        }
        header_part += "\r\n";
        sock.write_str(firstline_part);
        sock.write_str(header_part);
        sock.write_str(resp.body);
    }

    void HTTPServer::add_handler(const std::string &url_pattern, HTTPMethod method, http_handler_func handler)
    {
        switch (method)
        {
        case HTTPMethod::GET:
            _GET_handlers.push_back({url_pattern, handler});
            break;
        case HTTPMethod::POST:
            _POST_handlers.push_back({url_pattern, handler});
            break;
        default:
            break;
        }
    }

    void HTTPServer::sock_send_404(TCPSocket &sock)
    {
    }

    void HTTPServer::sock_send_500(TCPSocket &sock)
    {
    }

    /**
     * HTTP body读取类，
     * 负责读取和读取超时相关实现
     */

    HTTPServerBodyReader::HTTPServerBodyReader(TCPSocket &sock, HTTPMsg &msg)
        : _sock(sock), _msg(msg)
    {
        std::string LEN = "Content-Length";
        std::size_t body_len = 0;
        if (msg.headers.find(LEN) != msg.headers.end())
        {
            long blen = ::atol(msg.headers.at(LEN).c_str());
            if (blen < 0)
            {
                throw HTTPFormatException("HTTP body length error", msg.headers.at(LEN));
            }
            _content_length = blen;
        }
    }

    HTTPServerBodyReader::~HTTPServerBodyReader()
    {
    }

    std::string HTTPServerBodyReader::readn(std::size_t len)
    {
        size_t rn = len;
        if (_left_length > rn)
        {
            _left_length -= rn;
        }
        else
        {
            rn = _left_length;
            _left_length = 0;
        }
        return "";
    }

    std::string HTTPServerBodyReader::readall()
    {
        size_t rn = _left_length;
        _left_length = 0;
        // return std::move(_sock.readn(rn));
        return "";
    }

    std::size_t HTTPServerBodyReader::content_length()
    {
        return _content_length;
    }

    std::size_t HTTPServerBodyReader::left_length()
    {
        return 0;
    }

} // namespace ylib
