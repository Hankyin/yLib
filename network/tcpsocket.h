#pragma once

#include "abstractsocket.h"

namespace ylib
{
    //TCP通信类
    class TCPSocket : public AbstractSocket
    {
    public:
        TCPSocket();
        TCPSocket(int sockfd, const std::string &peerip = "", uint16_t peerport = 0);
        TCPSocket(const TCPSocket &) = delete;
        TCPSocket(TCPSocket &&);
        TCPSocket &operator=(const TCPSocket &) = delete;
        TCPSocket &operator=(TCPSocket &&);

        ~TCPSocket();
        void connect(const std::string &ip, uint16_t port);
        void close();
        std::string peerIP() const { return _peerIP; }
        uint16_t peerPort() const { return _peerPort; }

    private:
        std::string _peerIP;
        uint16_t _peerPort;
    };

} // namespace ylib
