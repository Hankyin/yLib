#include "minizipper.h"
#include "minizip/zip.h"
#include "core/fileinfo.h"
#include "core/directory.h"

#include <iostream>

namespace ylib
{

	struct MiniZipper::Impl
	{
		zipFile _zfile;
	};

	MiniZipper::MiniZipper(const std::string &zipname, ZipFlags flag)
	{
		_zipname = zipname;
		_password.clear();
		_open = false;
		_flag = flag;
		_impl = new MiniZipper::Impl;
	}

	MiniZipper::MiniZipper(const std::string &zipname, const std::string &passwd, ZipFlags flag)
	{
		_zipname = zipname;
		_password = passwd;
		_open = false;
		_flag = flag;
		_impl = new MiniZipper::Impl;
	}

	MiniZipper::~MiniZipper()
	{
		close();
		delete _impl;
	}

	int MiniZipper::open()
	{
		if (_open)
		{
			return -1;
		}
		ylib::FileInfo fi(_zipname);
		if (fi.is_exists() && _flag == ZipFlags::Append)
		{
			_impl->_zfile = zipOpen64(_zipname.c_str(), APPEND_STATUS_ADDINZIP);
		}
		else
		{
			_impl->_zfile = zipOpen64(_zipname.c_str(), APPEND_STATUS_CREATE);
		}
		if (_impl->_zfile)
		{
			_open = true;
			return 0;
		}
		else
		{
			_open = false;
			return -1;
		}
	}

	int MiniZipper::close()
	{
		if (!_open)
		{
			return -1;
		}

		if (_impl->_zfile != NULL)
		{
			zipClose(_impl->_zfile, NULL);
			_impl->_zfile = NULL;
		}
		_open = false;
		return 0;
	}

	bool MiniZipper::add(std::istream &source, const tm &timestamp, const std::string &nameInZip, ZipFlags flags)
	{
		if (!_impl->_zfile)
		{
			return false;
		}
		int compressLevel = 0;
		int zip64 = 0;
		int size_buf = 8096;
		int err = ZIP_OK;
		unsigned long crcFile = 0;
		zip_fileinfo zi = {0};
		zi.tmz_date.tm_sec = timestamp.tm_sec;
		zi.tmz_date.tm_min = timestamp.tm_min;
		zi.tmz_date.tm_hour = timestamp.tm_hour;
		zi.tmz_date.tm_mday = timestamp.tm_mday;
		zi.tmz_date.tm_mon = timestamp.tm_mon;
		zi.tmz_date.tm_year = timestamp.tm_year;

		size_t size_read;

		std::vector<char> buff;
		buff.resize(size_buf);

		if (nameInZip.empty())
			return false;

		if (flags & ZipFlags::Faster)
			compressLevel = 1;
		if (flags & ZipFlags::Better)
			compressLevel = 9;

		zip64 = is_large_file(source);
		if (_password.empty())
			err = zipOpenNewFileInZip64(_impl->_zfile,
										nameInZip.c_str(),
										&zi,
										NULL,
										0,
										NULL,
										0,
										NULL /* comment*/,
										(compressLevel != 0) ? Z_DEFLATED : 0,
										compressLevel,
										zip64);
		else
		{
			file_crc(source, buff, crcFile);
			err = zipOpenNewFileInZip3_64(_impl->_zfile,
										  nameInZip.c_str(),
										  &zi,
										  NULL,
										  0,
										  NULL,
										  0,
										  NULL /* comment*/,
										  (compressLevel != 0) ? Z_DEFLATED : 0,
										  compressLevel,
										  0,
										  /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
										  -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
										  _password.c_str(),
										  crcFile,
										  zip64);
		}

		if (ZIP_OK == err)
		{
			do
			{
				err = ZIP_OK;
				source.read(buff.data(), buff.size());
				size_read = (size_t)source.gcount();
				if (size_read < buff.size() && !source.eof() && !source.good())
					err = ZIP_ERRNO;

				if (size_read > 0)
					err = zipWriteInFileInZip(_impl->_zfile, buff.data(), (unsigned int)size_read);

			} while ((err == ZIP_OK) && (size_read > 0));
		}
		// else
		// 	throw EXCEPTION_CLASS(("Error adding '" + nameInZip + "' to zip").c_str());

		if (ZIP_OK == err)
			err = zipCloseFileInZip(_impl->_zfile);

		return ZIP_OK == err;
	}

	bool MiniZipper::add(std::istream &source, const std::string &nameInZip, ZipFlags flags)
	{
		return add(source, {}, nameInZip, flags);
	}

	bool MiniZipper::add(const std::string &fileOrFolderPath, ZipFlags flags)
	{
		bool ret = false;
		ylib::FileInfo fi;
		fi.set_path(fileOrFolderPath);
		if (fi.is_dir())
		{
			Directory dir(fi.path());
			auto dir_item = dir.get_filelist(Directory::NoDotAndDotDot);
			for (auto &&it : dir_item)
			{
				ret = add(fi.path() + '/' + it, flags);
				if (ret == false) //压缩失败，立刻返回。
				{
					break;
				}
			}
		}
		else
		{
			std::ifstream input(fileOrFolderPath, std::ios::binary);
			ret = add(input, fileOrFolderPath, flags);
			input.close();
		}
		return ret;
	}

	//辅助计算函数
	int MiniZipper::is_large_file(std::istream &input_stream)
	{
		ZPOS64_T pos = 0;
		input_stream.seekg(0, std::ios::end);
		pos = input_stream.tellg();
		input_stream.seekg(0);

		return (int)(pos >= 0xffffffff);
	}

	void MiniZipper::file_crc(std::istream &input_stream, std::vector<char> &buff, unsigned long &result_crc)
	{
		unsigned long calculate_crc = 0;
		unsigned long size_read = 0;
		unsigned long total_read = 0;

		do
		{
			input_stream.read(buff.data(), buff.size());
			size_read = (unsigned long)input_stream.gcount();

			if (size_read > 0)
				calculate_crc = crc32(calculate_crc, (const unsigned char *)buff.data(), size_read);

			total_read += size_read;

		} while (size_read > 0);

		input_stream.clear();
		input_stream.seekg(0, std::ios_base::beg);
		result_crc = calculate_crc;
	}

} // namespace ylib
