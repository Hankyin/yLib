#include "tcpsocket.h"
#include "sockexcept.h"
#include "core/logger.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

namespace ylib
{

    TCPSocket::TCPSocket()
    {
        _fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (_fd == -1)
        {
            throw SocketCreateException(::strerror(errno), errno);
        }
    }

    //从外界传入一个socket。
    TCPSocket::TCPSocket(int sockfd, const std::string &peerip, uint16_t peerport)
    {
        _fd = sockfd;
        _peerIP = peerip;
        _peerPort = peerport;
    }

    TCPSocket::TCPSocket(TCPSocket &&another)
    {
        if (another._fd == -1)
        {
            LOG_ERROR << "right move fail: socket fd is -1";
            return;
        }
        _fd = another._fd;
        _peerIP = another._peerIP;
        _peerPort = another._peerPort;
        another._fd = -1;
        another._peerIP.clear();
        another._peerPort = 0;
    }
    TCPSocket &TCPSocket::operator=(TCPSocket &&another)
    {
        //验证是否为自身
        if (this == &another)
        {
            return *this;
        }
        _fd = another._fd;
        _peerIP = another._peerIP;
        _peerPort = another._peerPort;
        another._fd = -1;
        another._peerIP.clear();
        another._peerPort = 0;
        return *this;
    }

    TCPSocket::~TCPSocket()
    {
        close();
    }

    void TCPSocket::connect(const std::string &ip, uint16_t port)
    {
        struct sockaddr_in addr;
        ::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(port);
        if (::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1)
        {
            throw SocketIPException(::strerror(errno), errno);
        }

        if (::connect(_fd, (sockaddr *)&addr, sizeof(addr)) != 0)
        {
            throw SocketConnectException(::strerror(errno), errno);
        }
        _peerIP = ip;
        _peerPort = port;
    }
    void TCPSocket::close()
    {
        if (_fd != -1)
        {
            LOG_TRACE << "tcp socket close";
            ::close(_fd);
            _fd = -1;
        }
    }

} // namespace ylib
