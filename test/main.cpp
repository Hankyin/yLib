#include <iostream>
#include "core/logger.h"
#include "network/tcpserver.h"

int main()
{
    LOG_LEVEL = ylib::LogMsg::TRACE;
    LOG_TRACE << "开始";
    return 0;
}