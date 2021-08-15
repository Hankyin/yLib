#include "abstractsocket.h"
#include "network.h"

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace ylib
{
    AbstractSocket::AbstractSocket()
    {
        _fd = -1;
    }

    AbstractSocket::~AbstractSocket()
    {
    }

    int AbstractSocket::write_str(const std::string &data)
    {
        if (_fd < 0)
        {
            return 0;
        }
        int written = 0;
        int length = data.length();
        while (written < length)
        {
            ssize_t nw = ::write(_fd, data.data() + written, length - written);
            if (nw > 0)
            {
                written += static_cast<int>(nw);
            }
            else if (errno == EINTR)
            {
                throw SocketINTRException(::strerror(errno), errno);
            }
            else if (errno != EAGAIN)
            {
                throw SocketWriteException(::strerror(errno), errno);
            }
        }
        return written;
    }

    int AbstractSocket::write_uint32(uint32_t data)
    {
        if (_fd < 0)
        {
            return 0;
        }
        int length = sizeof(data);
        uint32_t net_data = ::htonl(data);
        int written = 0;
        while (written < length)
        {
            ssize_t nw = ::write(_fd, reinterpret_cast<const char *>(&net_data) + written, length - written);
            if (nw > 0)
            {
                written += static_cast<int>(nw);
            }
            else if (errno == EINTR)
            {
                throw SocketINTRException(::strerror(errno), errno);
            }
            else if (errno != EAGAIN)
            {
                throw SocketWriteException(::strerror(errno), errno);
            }
        }
        return written;
    }

    std::string AbstractSocket::read(int len)
    {
        void *buf = ::malloc(len);
        ssize_t nr = ::read(_fd, static_cast<char *>(buf), len);
        if (nr > 0)
        {
            std::string st((char *)buf, nr);
            ::free(buf);
            return st;
        }
        else if (nr == 0)
        {
            ::free(buf);
            throw SocketCloseException(::strerror(errno), errno);
        }
        else if (errno == EINTR)
        {
            ::free(buf);
            throw SocketINTRException(::strerror(errno), errno);
        }
        else if (errno != EAGAIN)
        {
            ::free(buf);
            throw SocketReadException(::strerror(errno), errno);
        }
        return "";
    }

    std::string AbstractSocket::readn(int len)
    {
        if (_fd < 0)
        {
            return std::string();
        }
        int nread = 0;
        void *buf = ::malloc(len);
        while (nread < len)
        {
            ssize_t nr = ::read(_fd, static_cast<char *>(buf) + nread, len - nread);
            if (nr > 0)
            {
                nread += static_cast<int>(nr);
            }
            else if (nr == 0)
            {
                ::free(buf);
                throw SocketCloseException(::strerror(errno), errno);
            }
            else if (errno == EINTR)
            {
                ::free(buf);
                throw SocketINTRException(::strerror(errno), errno);
            }
            else if (errno != EAGAIN)
            {
                ::free(buf);
                throw SocketReadException(::strerror(errno), errno);
            }
        }
        std::string st((char *)buf, nread);
        ::free(buf);
        return std::move(st);
    }

    uint32_t AbstractSocket::read_uint32()
    {
        int nread = 0;
        uint32_t buf;
        int len = sizeof(buf);
        while (nread < len)
        {
            ssize_t nr = ::read(_fd, reinterpret_cast<char *>(&buf) + nread, len - nread);
            if (nr > 0)
            {
                nread += static_cast<int>(nr);
            }
            else if (nr == 0)
            {
                throw SocketCloseException(::strerror(errno), errno);
            }
            else if (errno == EINTR)
            {
                throw SocketINTRException(::strerror(errno), errno);
            }
            else if (errno != EAGAIN)
            {
                throw SocketReadException(::strerror(errno), errno);
            }
        }
        buf = ::ntohl(buf);
        return buf;
    }

} // namespace ylib
