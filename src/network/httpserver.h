/**
 * @file httpserver.h
 * @author yin
 * @brief 一个简单的http服务器，基于阻塞socket开发。
 * @version 0.1
 * @date 2021-08-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include "httpmessage.h"
#include "tcpsocket.h"
#include "tcpserver.h"

#include <string>
#include <functional>
#include <vector>
#include <atomic> // 服务器状态

namespace ylib
{

    class HTTPServer
    {
    public:
        /**
        * @brief http 请求的处理函数
        * @arg req 输入，http请求对象，
        * @arg resp 输出，HTTP的消息，需要构造这个对象。
        *           resp.version http版本，默认1.1
        *           resp.code 状态码，默认200
        *           resp.code_line 状态描述，不赋值的话根据code自动生成。
        *           resp.body 消息体
        *           resp.callback 消息回调。当传输大文件，chunk模式时可以用它。设置了的话就不会自动计算Content—Length了。
        *           resp.data std::any类型，回调可以用它存储东西。
        * @arg arg TODO, restful风格的参数。
        */
        typedef std::function<void(const HTTPRequestMsg &req, HTTPResponseMsg &resp, std::vector<std::string> arg)> HTTP_handler_func;

        HTTPServer();
        ~HTTPServer();
        void close() { _server_status = 0; }

        /**
         * @brief 启动一个HTTP服务器。
         * 每当一个客户端连接，就创建一个线程。
         *  TODO 使用线程池进行处理。 
         * 函数内部有一个死循环，调用后线程会被该函数阻塞，直到服务器被关闭，函数才会返回。       
         *          
         * @param ip 
         * @param port 
         * @param backlog 
         */
        void run(const std::string &ip, uint16_t port, int backlog = 10);

        /**
         * @brief 增加一个URL的处理器。
         * URL支持多种匹配模式
         * 1. 固定URL字符串
         * 2. restful风格，/user/{uid}/exec TODD
         * 3. 通配符 /static/* 
         * @param url_pattern http的url模板
         * @param method 
         * @param handler URL的处理器。
         * @param req_callback 
         */
        void add_handler(const std::string &url_pattern, HTTPMethod method, HTTP_handler_func handler, HTTPMsg::HTTP_msg_callback req_callback = nullptr);

        /**
         * @brief 设置_pack_size，即每次从socket中读取的最大数据包大小。
         * 
         * @param si 
         */
        void set_one_pack_size(size_t si) { _pack_size = si; }

        /**
        * @brief 设置
        * 
        * @param pack_num  最小是1，否则失败。
        * @return true 设置成功
        * @return false 设置失败
        */
        bool set_header_max_pack(int pack_num)
        {
            if (pack_num > 0)
            {
                _header_pack_num = pack_num;
                return true;
            }
            else
            {
                return false;
            }
        }

    private:
        struct Handler
        {
            std::string url_pattern;
            HTTP_handler_func handler;
            HTTPMsg::HTTP_msg_callback req_callback = nullptr;
        };
        void process(TCPSocket sock); // 每个客户端连接后的处理函数

        /**
         * @brief url的路由解析函数，如果解析失败，会抛出异常 TODO
         * 
         * @param req 需要被路由解析的请求。
         * @return Handler&  返回对应的处理器Handler
         * @exception HTTPRouteException 路由解析异常，如果找不到url，会抛出这个异常。TODO
         */
        Handler &url_router(const HTTPRequestMsg &req);
        
        /**
         * @brief 读取头部消息
         * 
         * @param req 
         * @param sock 
         * @param http_buf 
         * @return size_t 返回值是body_boundary
         */
        size_t sock_recv_req_head(HTTPRequestMsg &req, TCPSocket &sock, std::string &http_buf);

        /**
         * @brief 读取请求的消息体
         * 
         * @param req 
         * @param sock 
         * @param http_buf 
         * @param body_boundary 
         * @return size_t 返回值是在http_buf中本req中结束位置的索引。
         */
        size_t sock_recv_req_body(HTTPRequestMsg &req, TCPSocket &sock, std::string &http_buf, size_t body_boundary);

        void sock_send_resp(HTTPResponseMsg &resp, TCPSocket &sock);
        void sock_send_404(TCPSocket &sock);
        void sock_send_500(TCPSocket &sock);

        size_t _pack_size = 4096;
        uint32_t _header_pack_num = 1;
        std::atomic_int _server_status; // 服务器状态 0 停止， 1 运行
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
