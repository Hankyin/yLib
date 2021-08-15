#pragma once
#include <string>

#define APP ylib::Application::instance()

namespace ylib
{
    class Application
    {
    public:
        static Application &instance()
        {
            static Application ins;
            return ins;
        }
        static std::string &get_exe_path();

    private:
        Application();
        ~Application();
    };

} // namespace ylib