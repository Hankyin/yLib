#include <iostream>
#include "core/logger.h"
#include "network/httpclient.h"
#include "network/httpserver.h"

void httpclient();
void httpserver();

int main()
{
    LOG_LEVEL = ylib::LogMsg::TRACE;
    LOG_TRACE << "开始";
    httpserver();
    return 0;
}

void httpclient()
{
    std::string url = "47.94.249.180";
    uint16_t port = 14040;
    ylib::HTTPClient cli;
    ylib::HTTPRequestMsg req;
    ylib::HTTPResponseMsg resp;

    req.method = ylib::HTTPMethod::GET;
    req.headers["Host"] = "robot.com";

    req.printf();
    cli.connect(url, port);
    cli.send_req(req);
    cli.recv_resp(resp);
    cli.close();
    resp.printf();
}

void httpserver()
{
    ylib::HTTPServer server;
    std::string ip = "0.0.0.0";
    uint16_t port = 1234;
    server.add_handler("/test", ylib::HTTPMethod::GET,
                       [](const ylib::HTTPRequestMsg &req, ylib::HTTPResponseMsg &resp, std::vector<std::string> arg)
                       {
                           req.printf();
                           resp.body = "<h1>hello</h1>";
                       });

    server.run(ip, port);
}