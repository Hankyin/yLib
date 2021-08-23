#pragma once

/**
 * @file httpclient.h
 * @author yin 
 * @brief http请求客户端。
 * @details url和http没有必然联系，在发起http请求前需要先使用URL等类解析相关信息，然后构造http请求。
 *          当然本类也提供了一些方便函数集成了url解析。
 * @version 0.1
 * @date 2021-05-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "httpmessage.h"
#include "tcpsocket.h"

#include <string>
#include <stdexcept>

namespace ylib
{

    class HTTPClient
    {
    public:
        HTTPClient();
        ~HTTPClient();

        void connect(const std::string &ip, uint16_t port);
        void make_request(const HTTPRequestMsg &req, HTTPResponseMsg &resp);
        void close();

        /**
         * @brief 设置_pack_size，即每次从socket中读取的最大数据包大小。
         * 
         * @param si 
         */
        void set_one_pack_size(size_t si) { _pack_size = si; }

        /**
         * @brief 设置header_pack_num,即最多有 
         * 
         * @param pack_num 
         */
        void set_header_max_pack(uint32_t pack_num) { _header_pack_num = pack_num; }

        //方便使用的几个简单接口
        static HTTPResponseMsg GET(const std::string &url);
        static HTTPResponseMsg POST(const std::string &url, const std::string &body);

    private:
 
        TCPSocket _socket;
        void send_req(const HTTPRequestMsg &req);
        void recv_resp(HTTPResponseMsg &resp);
        size_t _pack_size = 4096;
        size_t _header_pack_num = 1;
        //0 断开
        //1 连接
        int _stat = 0;
    };

} // namespace ylib
