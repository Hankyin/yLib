#pragma once

#include <string>
#include <map>
#include <functional>

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
    };

    std::string HTTPCode_to_str(int stat_code);
    std::string HTTPMethod_to_str(HTTPMethod m);
    HTTPMethod str_to_HTTPMethod(const std::string &str);

    struct HTTPMsg
    {
        std::string first_line;
        std::map<std::string, std::string> headers;
        std::string body;

        int tag = 0; //用来标记这个msg的一些属性，比如是系统自动发送的，就置1.

        /**
         * @brief 将HTTP消息输出到指定路径的文件中。
         *        ifstream必须能够打开该文件。
         * 
         * @param path 文件路径，如果为空，则调用std::cout输出到屏幕
         */
        void printf(const std::string &path = "") const;
    };

    struct HTTPResponseMsg : public HTTPMsg
    {
        HTTPVersion version;
        int code;
        std::string code_line;
    };

    struct HTTPRequestMsg : public HTTPMsg
    {
        HTTPVersion version;
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

        /**
         * @brief 解析http的首行
         *        
         * @param msg_line 输出参数，外部提供空间，解析成功后结果放在这里
         * @param buf 要解析的字符串
         * @param start_pos 解析字符串的开始位置。
         * @return int 成功返回首行在字符串的结束位置，失败返回std::npos
         */
        int parser_firstline(std::string &msg_line, const std::string &buf, size_t start_pos = 0);

        /**
         * @brief 解析http的头部信息。buf中的头部信息必须完整，如果只包含部分头部信息认为解析失败，
         *        只有解析成功后msg_header才会被赋值。
         * @param msg_line 输出参数，外部提供空间，解析成功后结果放在这里
         * @param buf 要解析的字符串
         * @param start_pos 解析字符串的开始位置。
         * @return int 成功返回首行在字符串的结束位置，失败返回std::npos
         */
        int parser_header(std::map<std::string, std::string> &msg_header, const std::string &buf, size_t start_pos = 0);

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

        void set_firstline_max_size(size_t si) { _firstline_max_size = si; }
        void set_header_max_size(size_t si) { _header_max_size = si; }

    protected:
        size_t _firstline_max_size = 1024;
        size_t _header_max_size = 2048;
        static const std::string CRLF;
    };

} // namespace ylib
