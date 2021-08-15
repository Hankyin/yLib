#pragma once

/**
 * @brief 一个简单的HTTP服务器，阻塞式socket，每个请求创建一个线程。
 * 
 */

#include "httpmessage.h"
#include "tcpsocket.h"
#include "tcpserver.h"

#include <string>
#include <functional>
#include <vector>

namespace ylib
{
    class HTTPServerBodyReader
    {
    public:
        HTTPServerBodyReader(TCPSocket &sock, HTTPMsg &msg);
        ~HTTPServerBodyReader();
        std::string readn(std::size_t len);
        std::string readall();
        std::size_t content_length();
        std::size_t left_length();

    private:
        std::size_t _content_length;
        std::size_t _left_length;
        TCPSocket &_sock;
        HTTPMsg &_msg;
    };

    class HTTPServer
    {
    public:
        typedef std::function<void(const HTTPRequestMsg &req, HTTPResponseMsg &resp, HTTPServerBodyReader &body, std::vector<std::string> arg)> http_handler_func;
        
        HTTPServer();
        ~HTTPServer();
        int run(const std::string &ip, uint16_t port, int backlog = 10);
        void add_handler(const std::string &url_pattern, HTTPMethod method, http_handler_func handler);

    private:
        struct Handler
        {
            std::string url_pattern;
            http_handler_func handler;
        };
        void process(TCPSocket sock); // 每个客户端连接后的处理函数
        void make_response(HTTPRequestMsg &req, HTTPResponseMsg &resp, TCPSocket &sock);
        void exec_handler(HTTPRequestMsg &req, HTTPResponseMsg &resp, HTTPServerBodyReader &body, std::vector<std::string> arg);
        void sock_send_resp(HTTPResponseMsg &resp, TCPSocket &sock);
        void sock_send_404(TCPSocket &sock);
        void sock_send_500(TCPSocket &sock);

        HTTPResponseMsg http_url_router();
        int _server_status = 0; // 服务器状态 0 停止， 1 运行
        TCPServer _server;
        std::vector<Handler> _GET_handlers;
        std::vector<Handler> _POST_handlers;
        std::vector<Handler> _HEAD_handlers;
        std::vector<Handler> _PUT_handlers;
        std::vector<Handler> _DELETE_handlers;
        std::vector<Handler> _CONNECT_handlers;
        std::vector<Handler> _OPTIONS_handlers;
        std::vector<Handler> _TRACE_handlers;
        //为了避免竞态条件，禁止服务器启动后设置服务器名称（可能导致多线程访问，处理麻烦）
        std::string _server_name = "ylib-HTTPServer-sync/0.1";
    };

} // namespace ylib
