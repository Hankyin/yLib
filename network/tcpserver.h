#pragma once

#include "abstractsocket.h"
#include "tcpsocket.h"
#include <string>

namespace ylib
{

    //TCP服务端
    //不进行数据收发，当一个tcp客户端连接，会生成一个TCPSocket来进行通信。
    class TCPServer
    {
    public:
        TCPServer();
        ~TCPServer();
        void setopt_REUSEADDR();
        void bind(const std::string &ip, uint16_t port);
        void listen(int backlog = 10);
        TCPSocket accept();

    private:
        int _fd;
    };

} // namespace ylib
