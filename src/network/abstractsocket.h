#pragma once

#include <string>
#include <stdexcept>
#include <stdint.h>

namespace ylib
{


    //socket抽象类，主要负责数据的读写部分，udpsocket，tcpsocket，都继承该类
    class AbstractSocket
    {
    public:
        int write_str(const std::string &data);
        int write_uint32(uint32_t data);
        std::string read(int len);
        std::string readn(int len);
        uint32_t read_uint32();

    protected:
        AbstractSocket();
        ~AbstractSocket();
        int _fd;
    };

} // namespace ylib
