
#include "directory.h"
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

namespace ylib
{
    Directory::Directory()
    {
    }

    Directory::Directory(const std::string &dir_path)
    {
        set_path(dir_path);
    }

    Directory::~Directory()
    {
    }

    void Directory::set_path(const std::string &dir_path)
    {
        _dir_fi.set_path(dir_path);
    }

    const std::list<std::string> &Directory::get_filelist(int filter)
    {
        if ((_dir_fi.last_change_time() > _modifytime) || (filter != _last_filter))
        {
            //发现目录被修改，更新filelist
            DIR *dirp;
            struct dirent *dp;
            dirp = opendir(_dir_fi.path().c_str());
            while ((dp = readdir(dirp)) != NULL)
            {
                std::string f_name = dp->d_name;
                switch (dp->d_type)
                {
                case DT_REG:
                    if (filter & Filter::NoFile)
                        continue;
                case DT_DIR:
                    if (filter & Filter::NoDir)
                        continue;
                    if ((filter & Filter::NoHidden) && (f_name.front() == '.'))
                        continue;
                    if ((filter & Filter::NoDot) && (f_name == "."))
                        continue;
                    if ((filter & Filter::NoDotDot) && (f_name == ".."))
                        continue;
                default:
                    break;
                }
                _filelist.push_back(dp->d_name);
            }
            closedir(dirp);
        }
        return _filelist;
    }

    bool Directory::is_dir()
    {
        return _dir_fi.is_dir();
    }

    FileInfo &Directory::info()
    {
        return _dir_fi;
    }

    void cd(const std::string &new_path)
    {
    }

    int Directory::rename(const std::string &old_name, const std::string &new_name)
    {
        return ::rename(old_name.c_str(), new_name.c_str());
    }

    int Directory::mkpath(const std::string &new_path)
    {
        FileInfo dir_fi(new_path);
        if (dir_fi.is_dir())
        {
            return 0;
        }
        if (FileInfo(dir_fi.parent_path()).is_dir()) //父路径存在，直接mkdir
        {
            return ::mkdir(new_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
        auto dir_list = dir_fi.path_list();
        std::string cur_path;
        for (auto &&p : dir_list)
        {
            cur_path.push_back('/');
            cur_path += p;
            FileInfo t_fi(cur_path);
            if (!t_fi.is_dir())
            {
                int r = ::mkdir(cur_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                if (r == -1)
                {
                    perror("ylib::Directory::mkdir ");
                    return -1;
                }
            }
        }
        return 0;
    }

    int Directory::remove(const std::string &file_name)
    {
        return ::remove(file_name.c_str());
    }

    static int rm_r_p(const std::string &path_name)
    {
        Directory dir(path_name);
        auto list = dir.get_filelist(Directory::NoDotAndDotDot);
        for (auto &&l : list)
        {
            std::string sub_name = path_name;
            if (path_name.back() != '/')
            {
                sub_name.push_back('/');
            }
            sub_name += l;

            FileInfo fi(sub_name);
            if (fi.is_dir())
            {
                if (rm_r_p(sub_name) != 0)
                {
                    return -1;
                }
            }
            if (Directory::remove(sub_name) != 0)
            {
                return -1;
            }
        }
        return 0;
    }

    int Directory::rm_recursive(const std::string &path_name, bool keep_basedir)
    {
        int ret = rm_r_p(path_name);
        if (ret == 0 && !keep_basedir)
        {
            return remove(path_name);
        }
        return ret;
    }

} // namespace ylib
