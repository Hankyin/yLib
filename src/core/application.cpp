#include "application.h"
#include <unistd.h>

namespace ylib
{

    Application::Application()
    {
    }

    Application::~Application()
    {
    }

    std::string &Application::get_exe_path()
    {
        static std::string exe_path;
        if (exe_path.empty())
        {
            constexpr int max_size = 4096;
            char current_absolute_path[max_size];
            //获取当前程序绝对路径
            size_t bf_size = readlink("/proc/self/exe", current_absolute_path, max_size);
            if (bf_size < 0 || bf_size >= max_size)
            {
                perror("get exe path error:");
                return exe_path;
            }
            std::string fname = current_absolute_path;
            size_t sz = fname.find_last_of('/');
            exe_path = fname.substr(0, sz + 1);
        }

        return exe_path;
    }

} // namespace ylib
