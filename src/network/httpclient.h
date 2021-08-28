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

    /**
     * @brief HTTP请求客户端。输入req，返回resp。发生错误时抛出异常。
     *  1 创建好对象后，首先连接服务器。
     *  2 构建request对象，然后send req
     *  3 创建一个空的response对象，然后调用recv_resp，将resp填满。
     *  4 重复2、3，完成所有的http请求。
     *  5 关闭连接。
     * 
     *  客户端发送大文件
     *  当req的内容较少时直接将数据放在req的body中即可。
     *  当req的内容较多时，可以设置req的callback，在回调函数中进行处理。
     *  回调函数：bool (*)(HTTPMsg &msg, std::string &chunk_data);
     *   msg是指当前HTTP消息，chunk_data 是指将被传输的数据。返回真时代表还有剩余内容，返回假时认为调用结束。
     * 
     *  客户端接收大文件
     *  当resp的内容较多时，可以设置resp的callback，在回调函数中进行处理。
     *  回调函数：bool (*)(HTTPMsg &msg, std::string &chunk_data);
     *   msg是指当前HTTP消息，chunk_data 是指将本次被读取的数据。此时返回值无所谓，不会被处理。
     */
    class HTTPClient
    {
    public:
        HTTPClient();
        ~HTTPClient();

        void connect(const std::string &ip, uint16_t port);
        void send_req(const HTTPRequestMsg &req);
        void recv_resp(HTTPResponseMsg &resp);
        void close();

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

        //方便使用的几个简单接口
        static HTTPResponseMsg GET(const std::string &url);
        static HTTPResponseMsg POST(const std::string &url, const std::string &body);

    private:
        TCPSocket _socket;

        size_t _pack_size = 4096;
        uint32_t _header_pack_num = 1;
        //0 断开
        //1 连接
        int _stat = 0;
    };

} // namespace ylib
