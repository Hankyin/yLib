#pragma once

// C++
#include <string>
#include <map>
#include <functional>
#include <any> // 回调函数传参

namespace ylib
{

    enum class HTTPMethod
    {
        GET,
        POST,
        HEAD,
        PUT,
        DELETE,
        CONNECT,
        OPTIONS,
        TRACE,
        UNKNOW,
    };

    /**
     * @brief HTTP的版本，目前只支持1.1,其余todo
     * 
     */
    enum class HTTPVersion
    {
        V0_9,
        V1_0,
        V1_1,
        V2_0,
        V_UNKNOW,
    };

    std::string HTTPCode_to_str(int stat_code);
    std::string HTTPMethod_to_str(HTTPMethod m);
    std::string HTTPVersion_to_str(HTTPVersion v);
    HTTPMethod str_to_HTTPMethod(const std::string &str);
    HTTPVersion str_to_HTTPVersion(const std::string &str);

    struct HTTPMsg
    {
        /**
         * @brief HTTP msg 的回调函数，
         * @arg msg 当前的msg上下文
         * @arg chunk_data 每次传入/传出的数据。
         * @arg stat 内外传输的一个状态标志。
         * 
         */
        using HTTP_msg_callback = void (*)(HTTPMsg &msg, std::string &chunk_data, int& stat);
        std::string first_line;
        std::map<std::string, std::string> headers;
        std::string body;
        std::any data;
        HTTP_msg_callback callback = nullptr;

        int tag = 0; //用来标记这个msg的一些属性，比如是系统自动发送的，就置1.

        /**
         * @brief 将HTTP消息输出到指定路径的文件中。
         *        ifstream必须能够打开该文件。
         * 
         * @param path 文件路径，如果为空，则调用std::cout输出到屏幕
         */
        void printf(const std::string &path = "") const;

        //常用的一些HTTP头。
        static const std::string ContentLength;
        static const std::string ContentEncoding;
        static const std::string Connection;
        static const std::string TransferEncoding;
    };

    struct HTTPResponseMsg : public HTTPMsg
    {
        HTTPVersion version = HTTPVersion::V1_1;
        int code = 200;
        std::string code_line;
    };

    struct HTTPRequestMsg : public HTTPMsg
    {
        HTTPVersion version = HTTPVersion::V1_1;
        HTTPMethod method;
        std::string path; // 单纯URL，不包含query部分
        std::map<std::string, std::string> querys;
    };

    /**
     * @brief http消息解析器。
     * 
     */
    class HTTPMsgParser
    {
    public:
        HTTPMsgParser();
        ~HTTPMsgParser();
        static const std::string CRLF;

        /**
         * @brief 解析msg中的首行和头部信息。body部分不解析。
         *        本函数认为空行是结束标准。不传入空行认为解析失败。
         * @param msg 
         * @param buf 
         * @param start_pos 
         * @exception 解析失败抛出HTTPFormatException异常。
         */
        void parser_msg(HTTPMsg &msg, const std::string &buf, size_t start_pos = 0);

        /**
         * @brief 解析http回复消息的首行
         * 
         * @param version 输出
         * @param code 输出
         * @param code_line 输出 
         * @param first_line 输入
         * @return int 成功返回0，失败会抛出异常 HTTPFormatException。
         */
        int parser_resp_line(HTTPVersion &version, int &code, std::string &code_line, const std::string &first_line);

        /**
         * @brief 解析http请求消息的首行
         * 
         * @param version 
         * @param method 
         * @param path 
         * @param querys 
         * @param first_line 
         * @return int 成功返回0 失败抛异常 HTTPFormatException。
         */
        int parser_req_line(HTTPVersion &version, HTTPMethod &method, std::string &path,
                            std::map<std::string, std::string> &querys,
                            const std::string &first_line);
    };

} // namespace ylib
