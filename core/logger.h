#pragma once
//一个简单的异步日志库
//分为前端和后端，前端负责日志消息的构造，后端负责将消息写入文件。
//前端工作在调用线程，后端位于独立的日志线程。
//前后端各自使用自己的缓冲，前端写满后交换前后端缓冲，或者超时后交换。
//这样可以使用尽可能降低使用日志造成的性能损失。调用日志前端的消耗仅仅是将消息写入队列。
//一个典型的生产者消费者模型
//LogBackend是日志记录后端，用于向终端/文件/网络写数据
//Logger做日志的异步记录器，会调用LogBackend做最后的日志记录。单例模式，只有一个。
//LogMsg 日志构造器，用于生成一条特定格式的日志，调用Logger进行记录,日志级别也在这里处理。
//LOG_DEBUG 之类的宏定义，调用LogStream，用于方便使用

//日志库使用方法
//1. 简单用法：使用方便的宏
//  LOGGGER.backend().setbackend(LogBackend::File) //设置log后端
//  LOG_LEVEL = LogMsg::DEBUG; //定义全局日志级别，默认INFO
//  LOG_DEBUG << "这是第一条日志" << 123 << 3.14;//这一行结束，这个日志对象就死亡了，析构中调用了endl
//
//2. 真实实现
// Logger log(__FILE__, __LINE__,LogStream::DEBUG);
// log<< "这是第一条日志" << 123 << 3.14 << LogStream::endl;//不写endl是不会把数据传到日志后端的。
//

#include "core/fileinfo.h"
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>
#include <vector>

#define LOGGER ylib::Logger::instance()
#define LOG_LEVEL ylib::LogMsg::global_log_level
//每个宏都会创建一个临时的日志流变量。
#define LOG_TRACE ylib::LogMsg(__FILE__, __LINE__, ylib::LogMsg::TRACE)
#define LOG_DEBUG ylib::LogMsg(__FILE__, __LINE__, ylib::LogMsg::DEBUG)
#define LOG_INFO ylib::LogMsg(__FILE__, __LINE__, ylib::LogMsg::INFO)
#define LOG_WARN ylib::LogMsg(__FILE__, __LINE__, ylib::LogMsg::WARN)
#define LOG_ERROR ylib::LogMsg(__FILE__, __LINE__, ylib::LogMsg::ERROR)
#define LOG_FATAL ylib::LogMsg(__FILE__, __LINE__, ylib::LogMsg::FATAL)


namespace ylib
{
    class LogMsg
    {
    public:
        enum LogLevel
        {
            TRACE, 
            DEBUG, 
            INFO,
            WARN,  // 预期目标实现，但是某些指标或状态不符合预期。
            ERROR, // 运行出现错误，预期目标没有实现。
            FATAL, // 可能导致程序崩溃的错误
            NUM_LOG_LEVELS, //最后一个项目可以用来测量枚举的数量。cool
        };
        static LogLevel global_log_level;
        static LogMsg &endl(LogMsg &);

        LogMsg(const std::string &src_file, int src_line, LogLevel level);
        ~LogMsg();

        //重载<<运算符
        LogMsg &operator<<(LogMsg &(*pf)(LogMsg &));
        LogMsg &operator<<(bool v);
        LogMsg &operator<<(short);
        LogMsg &operator<<(unsigned short);
        LogMsg &operator<<(int);
        LogMsg &operator<<(unsigned int);
        LogMsg &operator<<(long);
        LogMsg &operator<<(unsigned long);
        LogMsg &operator<<(long long);
        LogMsg &operator<<(unsigned long long);
        LogMsg &operator<<(float v);
        LogMsg &operator<<(double);
        LogMsg &operator<<(char v);
        // self& operator<<(signed char);
        // self& operator<<(unsigned char);
        LogMsg &operator<<(const char *str);
        LogMsg &operator<<(const std::string &v);

    private:
        std::string fmt(); //生成格式化的日志消息
        std::string _msg;
        std::string _file;
        int _line;
        LogLevel _level;
    };

    class LogBackend
    {
    public:
        enum Backend
        {
            TERMINAL = 1,
            FILE = 2,
            NETWORK = 4, //TODO
        };
        void set_backend(Backend backend) { _cur_backend = backend; }
        void set_file_log_dir(const std::string &log_path);
        void set_net_log_server(const std::string &ip, uint16_t port);
        int out(const std::string &msg);

    private:
        int terminal_out(const std::string &msg);
        int file_out(const std::string &msg);
        int network_out(const std::string &msg);
        Backend _cur_backend = Backend::TERMINAL;
        FileInfo _log_fi;
        std::string _ip;
        uint16_t _port;
    };

    LogBackend::Backend operator|(LogBackend::Backend a, LogBackend::Backend b);

    class Logger
    {
    public:
        static Logger &instance()
        {
            //C++11单例模式
            static Logger g;
            return g;
        }

        //禁用拷贝
        Logger(const Logger &) = delete;
        void operator=(const Logger &) = delete;

        void append(const std::string &msg);
        void set_buffer_size(uint32_t size) { _msg_buf_size = size; }
        LogBackend &backend() { return _backend; }

    private:
        Logger();
        ~Logger();
        void bg_thread_fun();
        void bg_thread_stop();
        std::mutex _mutex;
        std::condition_variable _cond;
        bool _bg_start = true;
        std::shared_ptr<std::thread> _bg_write_th;
        uint32_t _msg_buf_size = 10000;
        LogBackend _backend;
        std::shared_ptr<std::string> _msg_buffer_cur_sp;
        std::shared_ptr<std::string> _msg_buffer_next_sp;
        std::vector<std::shared_ptr<std::string>> _full_msgs;
    };

}; // namespace ylib