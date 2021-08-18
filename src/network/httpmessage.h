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
        HTTPMsgParser(HTTPMsg &msg);
        ~HTTPMsgParser();
        virtual bool parser(const std::string &buf, size_t start_idx = 0);

        void reset();
        void set_firstline_max_size(size_t si) { _firstline_max_size = si; }
        void set_header_max_size(size_t si) { _header_max_size = si; }

    protected:
        bool read_firstline();
        bool read_header();
        size_t _firstline_max_size;
        size_t _header_max_size;
        //http协议读取阶段
        // 0 头部
        // 1 header
        // 2 body,也就是本类的结束了
        int _parser_state = 0;
        size_t _header_start_pos = 0;
        size_t _body_start_pos = 0;
        HTTPMsg &_msg;
    };

    class HTTPReqParser : public HTTPMsgParser
    {
    public:
        HTTPReqParser(HTTPRequestMsg &req);
        ~HTTPReqParser();
        virtual bool parser() override;

    private:
        HTTPRequestMsg &_req;
    };

    class HTTPRespHeaderParser : public HTTPHeaderParser
    {
    public:
        HTTPRespHeaderParser(HTTPResponseMsg &resp);
        ~HTTPRespHeaderParser();
        virtual bool parser() override;

    private:
        HTTPResponseMsg &_resp;
    };

} // namespace ylib
