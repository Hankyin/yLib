#pragma once
#include <string>
#include <vector>
#include "datetime.h"
#include "sys/stat.h"

namespace ylib
{
    class FileInfo
    {
    public:
        FileInfo();
        FileInfo(const std::string &path);
        ~FileInfo();
        void set_path(const std::string &path);
        bool empty();
        //路径分析
        const std::string &path() const;
        const std::string &full_path();
        const std::string &parent_path();
        const std::string &filename();
        const std::string base_name();
        const std::string suffix();
        const std::vector<std::string> &path_list();
        bool is_absolute() const
        {
            return _is_abs_path;
        }

        //文件信息，调用stat获取
        //  所有权
        uid_t user_id();
        gid_t group_id();
        //  类型
        bool is_file();
        bool is_dir();
        bool is_char();
        bool is_block();
        bool is_fifo();
        bool is_sock();
        bool is_symlink();
        //  大小
        uint64_t size();
        //  日期
        DateTime last_access_time();
        DateTime last_modify_time();
        DateTime last_change_time();

        //权限,access获取
        bool is_exists();
        bool is_executable();
        bool is_readable();
        bool is_wriable();
       static bool is_exists(const std::string &path);
       static bool is_executable(const std::string &path);
       static bool is_readable(const std::string &path);
       static bool is_wriable(const std::string &path);
        //其他
        // const std::string symlink_target();
        void refresh(); //清空所有缓存的数据

    private:
        bool _is_abs_path = false;
        std::string _path;                   //用户设置的路径，绝对相对都可能。
        std::string _filename;               //完整的文件名
        std::string _parent;                 //父路径
        std::string _full_path;              //文件绝对路径
        std::vector<std::string> _path_list; //文件目录列表
        bool _stat_cache = false;
        struct stat _stat;
        void stat_check();
    };

} // namespace ylib