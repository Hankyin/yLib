#include "logger.h"
#include "datetime.h"
#include "directory.h"
#include <chrono>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>

using namespace std;
using namespace std::chrono;

namespace ylib
{

    const char *log_level_name[LogMsg::NUM_LOG_LEVELS] =
        {
            "[TRACE]",
            "[DEBUG]",
            "[INFO ]",
            "[WARN ]",
            "[ERROR]",
            "[FATAL]",

    };

    LogMsg::LogLevel LogMsg::global_log_level = LogMsg::INFO; //默认全局日志级别是info

    LogMsg::LogMsg(const std::string &src_file, int src_line, LogLevel level)
    {
        _file = src_file;
        _line = src_line;
        _level = level;
    }

    LogMsg::~LogMsg()
    {
        endl(*this);
    }

    std::string LogMsg::fmt()
    {
        // 2020-01-22 11:11:11 [INFO ]  main.cpp 123 一条日志
        std::string msg;
        if (_level >= global_log_level && !_msg.empty())
        {
            msg += ylib::DateTime::now().to_string();
            msg.push_back(' ');
            msg += log_level_name[_level];
            msg.push_back(' ');
            std::hash<std::thread::id> th_id_hash;
            msg += std::to_string(th_id_hash(std::this_thread::get_id())); //我们将线程id的哈希值作为线程唯一标识符
            msg.push_back(' ');
            // msg += _file;
            // msg.push_back(' ');
            // msg += std::to_string(_line);
            // msg.push_back(' ');
            msg += _msg;
            msg.push_back('\n');
        }
        return msg;
    }

    LogMsg &LogMsg::endl(LogMsg &imp)
    {
        if (!imp._msg.empty())
        {
            Logger::instance().append(imp.fmt());
            imp._msg.clear(); //清空消息,防止多次调用endl函数导致该条消息多次append到后端.
        }
        return imp;
    }

    LogMsg &LogMsg::operator<<(LogMsg &(*pf)(LogMsg &))
    {
        pf(*this);
        return *this;
    }

    LogMsg &LogMsg::operator<<(bool v)
    {
        _msg += v ? "true" : "false";
        return *this;
    }

    LogMsg &LogMsg::operator<<(short s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(unsigned short s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(int s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(unsigned int s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(long s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(unsigned long s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(long long s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(unsigned long long s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(float s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(double s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(char s)
    {
        _msg += std::to_string(s);
        return *this;
    }

    LogMsg &LogMsg::operator<<(const char *str)
    {
        if (str)
        {
            _msg += str;
        }
        else
        {
            _msg += "(null)";
        }
        return *this;
    }

    LogMsg &LogMsg::operator<<(const std::string &v)
    {
        _msg += v;
        return *this;
    }

    //****************************************************************************//
    //*********************         日志后端      *********************************//
    //****************************************************************************//
    LogBackend::Backend operator|(LogBackend::Backend a, LogBackend::Backend b)
    {
        return (LogBackend::Backend)((int)a | (int)b);
    }

    void LogBackend::set_file_log_dir(const std::string &log_path)
    {
        _log_fi.set_path(log_path);
    }

    void LogBackend::set_net_log_server(const std::string &ip, uint16_t port)
    {
        _ip = ip;
        _port = port;
    }

    int LogBackend::out(const std::string &msg)
    {
        int ret = 0;
        if (_cur_backend & Backend::TERMINAL)
        {
            ret += terminal_out(msg);
        }
        if (_cur_backend & Backend::FILE)
        {
            ret += file_out(msg);
        }
        if (_cur_backend & Backend::NETWORK)
        {
            ret += network_out(msg);
        }
        return ret == 0 ? 0 : -1;
    }

    int LogBackend::terminal_out(const std::string &msg)
    {
        std::clog << msg;
        return 0;
    }

    int LogBackend::file_out(const std::string &msg)
    {
        //查看是否指定了log文件名，没有指定就按照 日期.log 命名文件
        ofstream log_file_stream;
        std::string fn;
        fn = DateTime::now().date().to_string();
        fn += ".log";
        if (!_log_fi.empty())
        {
            if ((!_log_fi.is_exists()) && (Directory::mkpath(_log_fi.full_path())))
            {
                //目录不存在且创建也失败了。
                terminal_out("logger path error!");
                return -1;
            }
            fn = _log_fi.full_path() + "/" + fn;
        }
        log_file_stream.open(fn, ios::app);
        if (!log_file_stream.is_open())
        {
            terminal_out("logger open file fail!");
            return -1;
        }
        log_file_stream << msg;
        log_file_stream.close();
        return 0;
    }

    int LogBackend::network_out(const std::string &msg)
    {
        return 0;
    }

    //****************************************************************************//
    //*********************         日志记录器     *******************************//
    //****************************************************************************//

    Logger::Logger()
    {
        _bg_start = true;
        _msg_buffer_cur_sp = std::make_shared<std::string>();
        _msg_buffer_next_sp = std::make_shared<std::string>();
        _bg_write_th = std::make_shared<std::thread>([&]() { bg_thread_fun(); });
    }

    Logger::~Logger()
    {
        bg_thread_stop();
    }

    void Logger::append(const std::string &msg)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_msg_buffer_cur_sp->size() < _msg_buf_size) //前端缓冲未满
        {
            _msg_buffer_cur_sp->append(msg);
        }
        else
        {
            _full_msgs.push_back(_msg_buffer_cur_sp);
            _msg_buffer_cur_sp.reset();
            if (_msg_buffer_next_sp) //切换辅助缓冲。
            {
                std::swap(_msg_buffer_next_sp, _msg_buffer_cur_sp);
            }
            else
            {
                _msg_buffer_cur_sp = std::make_shared<std::string>(); //辅助缓冲也满了，创建一个新的。很少进入
            }

            _msg_buffer_cur_sp->append(msg);
            _cond.notify_one();
        }
    }

    void Logger::bg_thread_fun()
    {
        //备用的两个缓冲
        std::shared_ptr<std::string> backup_buf1 = std::make_shared<std::string>();
        std::shared_ptr<std::string> backup_buf2 = std::make_shared<std::string>();
        std::vector<std::shared_ptr<std::string>> buf_to_write;
        buf_to_write.reserve(16);
        while (_bg_start)
        {
            {
                std::unique_lock<std::mutex> u_lock(_mutex);
                if (_full_msgs.empty()) //条件变量非通常用法。
                {
                    _cond.wait_for(u_lock, seconds(1));
                }
                std::swap(buf_to_write, _full_msgs);
                buf_to_write.push_back(_msg_buffer_cur_sp);  //注意要添加当前buf，当前buf未满也要将其刷新写入。
                _msg_buffer_cur_sp = std::move(backup_buf1); //将备用缓冲交换给前端,注意是移动不是复制或赋值.
                if (!_msg_buffer_next_sp)
                {
                    _msg_buffer_next_sp = std::move(backup_buf2);
                }
            }
            if (buf_to_write.size() > 25)
            {
                //log太多，报错，保留前两条消息，抛弃其他消息。
                std::cout << "msg too large" << std::endl;
                buf_to_write.erase(buf_to_write.begin() + 2, buf_to_write.end());
            }
            for (auto &str : buf_to_write)
            {
                _backend.out(*str); //写入日志消息
            }
            if (buf_to_write.size() > 2)
            {
                //删掉多余的缓冲
                buf_to_write.resize(2);
            }

            //恢复两个缓冲,尽可能使用已经申请的内存.
            if (!backup_buf1) //每次循环，buf1必然会被交换给cur_buf，所以他必然为空
            {
                backup_buf1 = buf_to_write.back();
                buf_to_write.pop_back();
                backup_buf1->clear(); //这个只是重置了一下size的位置，没有真正的清空所有的字符串。
            }
            if (!backup_buf2) //buf2会被交换给next_buf，如果next_buf没有被使用，buf2就不为空
            {
                backup_buf2 = buf_to_write.back();
                buf_to_write.pop_back();
                backup_buf2->clear();
            }

            buf_to_write.clear();
        }
    }

    void Logger::bg_thread_stop()
    {
        _bg_start = false;
        if (_bg_write_th->joinable())
            _bg_write_th->join();
    }

}; // namespace ylib
