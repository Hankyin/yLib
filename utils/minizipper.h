#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <time.h>

// std::ifstream input1("some file");
// std::ifstream input2("some file");

// Zipper zipper("ziptest.zip");
// zipper.open();
// zipper.add(input1, "Test1");
// zipper.add(input2, "Test1");

// zipper.close();
namespace ylib
{

    class MiniZipper
    {
    public:
        enum ZipFlags
        {
            Overwrite = 0x01,
            Append = 0x02,
            Store = 0x04,
            Faster = 0x08,
            Better = 0x10
        };

        MiniZipper(const std::string &zipname, ZipFlags flag = ZipFlags::Overwrite);
        MiniZipper(const std::string &zipname, const std::string &passwd, ZipFlags flag = ZipFlags::Overwrite);
        ~MiniZipper();

        bool add(std::istream &source, const tm &timestamp, const std::string &nameInZip = std::string(), ZipFlags flags = Better);
        bool add(std::istream &source, const std::string &nameInZip = std::string(), ZipFlags flags = Better);
        bool add(const std::string &fileOrFolderPath, ZipFlags flags = Better);

        int open();
        int close();

    private:
        void release();
        int is_large_file(std::istream &input_stream);
        void file_crc(std::istream &input_stream, std::vector<char> &buff, unsigned long &result_crc);
        std::string _password;
        std::string _zipname;
        bool _open;
        ZipFlags _flag;
        // std::iostream &m_obuffer;
        // std::vector<unsigned char> &m_vecbuffer;
        bool m_usingMemoryVector;
        bool m_usingStream;
        bool m_open;

        struct Impl;
        Impl *_impl;
    };

} // namespace ylib