#include "tcpserver.h"
#include "sockexcept.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

namespace ylib
{
    TCPServer::TCPServer()
    {
        _fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (_fd == -1)
        {
            throw SocketCreateException(::strerror(errno), errno);
        }
    }

    TCPServer::~TCPServer()
    {
        if (_fd != -1)
        {
            ::close(_fd);
        }
    }
    void TCPServer::setopt_REUSEADDR()
    {
        int yes = 1;
        if (::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0)
        {
            throw SocketOptException(::strerror(errno), errno);
        }
    }
    void TCPServer::bind(const std::string &ip, uint16_t port)
    {
        struct sockaddr_in addr;
        ::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(port);
        if (::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1)
        {
            throw SocketIPException(::strerror(errno), errno);
        }
        if (::bind(_fd, (sockaddr *)&addr, sizeof(addr)))
        {
            throw SocketBindException(::strerror(errno), errno);
        }
    }

    void TCPServer::listen(int backlog)
    {
        if (::listen(_fd, 10) != 0)
        {
            throw SocketListenException(::strerror(errno), errno);
        }
    }

    TCPSocket TCPServer::accept()
    {
        struct sockaddr_in peer_addr;
        ::memset(&peer_addr, 0, sizeof(peer_addr));
        socklen_t addrlen = 0;
        int sockfd = ::accept(_fd, reinterpret_cast<struct sockaddr *>(&peer_addr), &addrlen);
        if (sockfd < 0)
        {
            throw SocketAcceptException(::strerror(errno), errno);
        }
        char ip_buf[INET_ADDRSTRLEN] = {0};
        std::string p_ip = ::inet_ntop(AF_INET, &peer_addr.sin_addr, ip_buf, INET_ADDRSTRLEN);
        uint16_t p_port = ::ntohs(peer_addr.sin_port);
        TCPSocket s(sockfd, p_ip, p_port);
        return std::move(s);
    }

} // namespace ylib
