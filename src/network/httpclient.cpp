#include "httpclient.h"
#include "netutils.h"
#include "httpmessage.h"
#include "httpexcept.h"

#include "core/stringhelper.h"

#include <iostream>
#include <limits.h>
namespace ylib
{

    HTTPResponseMsg HTTPClient::GET(const std::string &url_str)
    {
        std::string http_ip;
        URL _url(url_str);
        if (is_ipv4(_url.host))
        {
            http_ip = _url.host;
        }
        else
        {
            http_ip = resolve_hostname(_url.host);
        }
        HTTPClient cli;
        HTTPRequestMsg req;
        HTTPResponseMsg resp;
        req.method = HTTPMethod::GET;
        req.path = _url.path;
        cli.connect(http_ip, _url.port);
        cli.make_request(req, resp);
        return resp;
    }

    HTTPResponseMsg HTTPClient::POST(const std::string &url, const std::string &body)
    {
    }

    HTTPClient::HTTPClient()
    {
    }

    HTTPClient::~HTTPClient()
    {
    }

    const std::string HTTPClient::ContentLength = "Content-Length";
    const std::string HTTPClient::ContentEncoding = "Content-Encoding";
    const std::string HTTPClient::Connection = "Connection";
    const std::string HTTPClient::TransferEncoding = "Transfer-Encoding";

    void HTTPClient::connect(const std::string &ip, uint16_t port)
    {

        _socket.connect(ip, port);
        _stat = 1;
    }

    void HTTPClient::close()
    {
        _socket.close();
        _stat = 0;
    }

    void HTTPClient::send_req(const HTTPRequestMsg &req)
    {
        std::string head_line;
        std::string header_part;
        head_line = HTTPMethod_to_str(req.method);
        head_line += ' ';
        head_line += req.path;
        head_line += " HTTP/1.1\n";

        for (auto &&kv : req.headers)
        {
            std::string h;
            h = kv.first;
            h += ": ";
            h += kv.second;
            h += "\n";
            header_part += h;
        }
        header_part.push_back('\n');

        _socket.write_str(head_line);
        _socket.write_str(header_part);
        _socket.write_str(req.body);
    }

    void HTTPClient::recv_resp(HTTPResponseMsg &resp)
    {
        std::string http_buf;
        int st = 0;
        int pknum = 1;
        HTTPMsgParser msg_parser;
        std::string double_CRLF = HTTPMsgParser::CRLF + HTTPMsgParser::CRLF;

        while (true)
        {
            std::string pack = _socket.read(_pack_size);
            http_buf += pack;
            //首先判断头部消息是否完整，根据两个回车换行判断。
            //头部消息完整后再调用http解析器处理。
            size_t p = http_buf.find(double_CRLF, st);

            if (p == std::string::npos)
            {
                // 尚未找到头部结束标志
                pknum++;
                if (pknum > _header_pack_num)
                {
                    //过长报错
                    throw HTTPTooLongException(_header_pack_num, _pack_size);
                }
                else
                {
                    //更新st值，避免重复查找
                    st = http_buf.size() > 2 ? http_buf.size() - 2 : 0; // 避免httpbuf的size小于0
                    continue;
                }
            }
            else
            {
                //成功找到了头部的结尾
                std::string header_part = http_buf.substr(0, p);
                msg_parser.parser_msg(resp, header_part);
                msg_parser.parser_resp_line(resp.version, resp.code, resp.code_line, resp.first_line);

                //头部已经被解析完成，根据header的信息进行读取body等处理。
                // 目前支持Content-Length，暂不支持gzip，chunk等模式

                //Content-Length处理，对body进行处理
                if (resp.headers.find(ContentLength) == resp.headers.end())
                {
                    size_t body_len = getContentLength(resp.headers);
                    if(resp.callback == nullptr)
                    {
                        //默认处理
                        
                    }
                    else
                    {
                        //调用客户自己写的回调。
                    }
                }

                if (callback == nullptr)
                {
                    //没有设置回调，将body放在resp中。
                }
                else
                {
                    //设置了回调函数，则调用回调函数处理.
                }

                resp.body = rp.get_left_str(); //读取剩余的全部字符串
                size_t left_len = body_len - resp.body.size();
                if (left_len > 0)
                {
                    std::string left_body_content = _socket.readn(left_len);
                    resp.body += left_body_content;
                }
                break;
            }
        }
    }

    size_t HTTPClient::getContentLength(std::map<std::string, std::string> headers)
    {

        try
        {
            size_t body_len = 0;
            body_len = std::stoul(headers.at(ContentLength)); // http的长度是没有限制的，size_t能够放下吗？
            return body_len;
        }
        catch (const std::exception &e)
        {
            // 产生异常，认为解析失败。
            // 比如没有contentlength字段
            // 比如长度无法被解析。
            return 0;
        }
    }

} // namespace ylib
