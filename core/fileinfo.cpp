#include "fileinfo.h"
#include "stringhelper.h"
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

namespace ylib
{

    FileInfo::FileInfo()
    {
    }

    FileInfo::FileInfo(const std::string &path)
    {
        set_path(path);
    }

    FileInfo::~FileInfo()
    {
    }

    void FileInfo::set_path(const std::string &path)
    {
        _path = path;
        if (_path.front() == '/')
        {
            _is_abs_path = true;
        }
        else
        {
            _is_abs_path = false;
        }
    }

    bool FileInfo::empty()
    {
        return _path.empty();
    }

    const std::string &FileInfo::path() const
    {
        return _path;
    }

    const std::string &FileInfo::full_path()
    {
        if (!_path.empty() && _full_path.empty())
        {
            if (_path.front() == '/')
            {
                _full_path = _path;
            }
            else
            {
                char cur_path[PATH_MAX + 1] = {0};
                //当文件不存在时，realpath处理的有问题。
                // ::realpath(_path.c_str(), cur_path);
                ::getcwd(cur_path, PATH_MAX);
                std::string path_str = cur_path;
                if (_path == ".")
                {
                    _full_path = path_str;
                }
                else if (_path.substr(0, 2) == "./")
                {
                    _full_path = path_str + _path.substr(1);
                }
                else if (_path == "..")
                {
                    auto idx = path_str.find_last_of('/');
                    if (idx == path_str.npos) //这个正常不可能
                    {
                        _full_path.clear(); //意思就是错误，返回空的fullpath
                    }
                    _full_path = path_str.substr(0, idx);
                }
                else if (_path.substr(0, 3) == "../")
                {
                    auto idx = path_str.find_last_of('/');
                    if (idx == path_str.npos) //这个正常不可能
                    {
                        _full_path.clear(); //意思就是错误，返回空的fullpath
                    }
                    _full_path = path_str.substr(0, idx) + _path.substr(2);
                }
                else
                {
                    _full_path = path_str + '/' + _path;
                }
            }
        }
        return _full_path;
    }

    const std::string &FileInfo::parent_path()
    {
        if (_parent.empty())
        {
            filename();
        }
        return _parent;
    }

    const std::string &FileInfo::filename()
    {
        if (_filename.empty())
        {
            std::string fph = full_path();
            auto idx = full_path().find_last_of('/');
            if (idx == std::string::npos)
            {
                _filename = _path;
                _parent = _path;
            }
            else
            {
                _filename = fph.substr(idx + 1);
                _parent = fph.substr(0, idx);
            }
        }
        return _filename;
    }

    const std::string FileInfo::base_name()
    {
        std::string fnm = filename();
        auto idx = fnm.find_last_of('.');
        if (idx == fnm.npos)
        {
            return fnm;
        }
        else
        {
            return fnm.substr(0, fnm.size() - idx);
        }
    }

    const std::string FileInfo::suffix()
    {
        std::string fnm = filename();
        auto idx = fnm.find_last_of('.');
        if (idx == fnm.npos)
        {
            return "";
        }
        else
        {
            return fnm.substr(idx);
        }
    }

    const std::vector<std::string> &FileInfo::path_list()
    {
        if (_path_list.empty())
        {
            std::string fullp = full_path();
            // if(!fullp.empty())
            // {
            //     _path_list.push_back("/");
            // }
            size_t pos = 0;
            size_t len = fullp.length();
            while (pos < len)
            {
                int find_pos = fullp.find('/', pos);
                if (find_pos < 0)
                {
                    _path_list.push_back(fullp.substr(pos, len - pos));
                    break;
                }
                std::string el = fullp.substr(pos, find_pos - pos);
                if (!el.empty())
                    _path_list.push_back(el);
                pos = find_pos + 1;
            }
        }
        return _path_list;
    }

    //  所有权
    uid_t FileInfo::user_id()
    {
        stat_check();
        return _stat.st_uid;
    }

    gid_t FileInfo::group_id()
    {
        stat_check();
        return _stat.st_gid;
    }

    //  类型
    bool FileInfo::is_file()
    {
        stat_check();
        if (!_stat_cache)
        {
            return false;
        }
        return S_ISREG(_stat.st_mode);
    }
    bool FileInfo::is_dir()
    {
        stat_check();
        if (!_stat_cache)
        {
            return false;
        }
        return S_ISDIR(_stat.st_mode);
    }
    bool FileInfo::is_char()
    {
        stat_check();
        if (!_stat_cache)
        {
            return false;
        }
        return S_ISCHR(_stat.st_mode);
    }
    bool FileInfo::is_block()
    {
        stat_check();
        if (!_stat_cache)
        {
            return false;
        }
        return S_ISBLK(_stat.st_mode);
    }
    bool FileInfo::is_fifo()
    {
        stat_check();
        if (!_stat_cache)
        {
            return false;
        }
        return S_ISFIFO(_stat.st_mode);
    }
    bool FileInfo::is_sock()
    {
        stat_check();
        if (!_stat_cache)
        {
            return false;
        }
        return S_ISSOCK(_stat.st_mode);
    }
    bool FileInfo::is_symlink()
    {
        stat_check();
        if (!_stat_cache)
        {
            return false;
        }
        return S_ISLNK(_stat.st_mode);
    }

    //  权限
    //      权限太麻烦了，先不做了
    //  大小
    uint64_t FileInfo::size()
    {
        stat_check();
        if (!_stat_cache)
        {
            return 0;
        }
        return _stat.st_size;
    }

    DateTime FileInfo::last_access_time()
    {
        stat_check();
        if (!_stat_cache)
        {
            return 0;
        }
        return _stat.st_atime;
    }

    DateTime FileInfo::last_modify_time()
    {
        stat_check();
        if (!_stat_cache)
        {
            return 0;
        }
        return _stat.st_mtime;
    }

    DateTime FileInfo::last_change_time()
    {
        stat_check();
        if (!_stat_cache)
        {
            return 0;
        }
        return _stat.st_ctime;
    }

    void FileInfo::stat_check()
    {
        if (!_stat_cache)
        {
            if (::stat(full_path().c_str(), &_stat) == 0)
            {
                _stat_cache = true;
            }
        }
    }
    bool FileInfo::is_exists()
    {
        return FileInfo::is_exists(full_path());
    }

    bool FileInfo::is_executable()
    {
        return FileInfo::is_exists(full_path());
    }

    bool FileInfo::is_readable()
    {
        return FileInfo::is_exists(full_path());
    }

    bool FileInfo::is_wriable()
    {
        return FileInfo::is_exists(full_path());
    }

    bool FileInfo::is_exists(const std::string &path)
    {
        return ::access(path.c_str(), F_OK) == 0;
    }

    bool FileInfo::is_executable(const std::string &path)
    {
        return ::access(path.c_str(), X_OK) == 0;
    }

    bool FileInfo::is_readable(const std::string &path)
    {
        return ::access(path.c_str(), R_OK) == 0;
    }

    bool FileInfo::is_wriable(const std::string &path)
    {
        return ::access(path.c_str(), W_OK) == 0;
    }

    void FileInfo::refresh()
    {
        _is_abs_path = false;
        _path.clear();
        _filename.clear();
        _full_path.clear();
        _path_list.clear();
        _stat_cache = false;
    }
} // namespace ylib
