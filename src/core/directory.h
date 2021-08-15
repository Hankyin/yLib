//
//Directory, Linux下的目录操作，意图充当C++17中的filesystem。
//可以查找/删除/重命名/移动 目录和文件。
//
//

#include <string>
#include <list>
#include "fileinfo.h"

namespace ylib
{
    class Directory
    {
    public:
        enum Filter
        {
            NoFilter = 0,
            NoDir = 0x001,
            NoFile = 0x002,
            NoDot = 0x2000,
            NoDotDot = 0x4000,
            NoDotAndDotDot = NoDot | NoDotDot,
            NoHidden = 0x100,
        };
        Directory(const std::string &dir_path);
        Directory();
        ~Directory();
        void set_path(const std::string &dir_path);
        const std::list<std::string> &get_filelist(int filter = NoFilter);
        bool is_dir();
        FileInfo &info();
        //静态函数
        static void cd(const std::string &new_path);                                 //切换程序的当前目录
        static int rename(const std::string &old_name, const std::string &new_name); //
        static int mkpath(const std::string &new_path);
        static int remove(const std::string &file_name);
        static int rm_recursive(const std::string &path_name, bool keep_basedir = false); //递归删除指定目录,
        static Directory current();
        static bool set_current(const std::string &dir_path);

    private:
        int _last_filter;
        DateTime _modifytime;
        std::list<std::string> _filelist;
        FileInfo _dir_fi;
    };

} // namespace ylib
