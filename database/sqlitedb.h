#pragma once

#include "sqlite/sqlite3.h"
#include <string>
#include <vector>

namespace ylib
{
    //
    //对于sqlite异常类。
    //
    class SQLiteException : public std::exception
    {
    public:
        SQLiteException(int errcode) : _ecode(errcode) {}
        ~SQLiteException() {}
        virtual const char *what() const noexcept override
        {
            return err_str();
        }
        const char *err_str() const noexcept
        {
            return sqlite3_errstr(_ecode);
        }

    private:
        int _ecode;
    };

    class SQLiteConnException : public SQLiteException
    {
    public:
        using SQLiteException::SQLiteException;
        virtual const char *what() const noexcept override
        {
            return "sqlite connection fail";
        }
    };
    class SQLiteStmtException : public SQLiteException
    {
    public:
        using SQLiteException::SQLiteException;
        virtual const char *what() const noexcept override
        {
            return "sqlite stmt init fail";
        }
    };

    class SQLiteStepException : public SQLiteException
    {
    public:
        using SQLiteException::SQLiteException;
        virtual const char *what() const noexcept override
        {
            return "sqlite stmt step exec fail";
        }
    };
    //
    //对于sqlite的一个C++封装
    //

    class SQLiteDB
    {
    public:
        SQLiteDB(const std::string &db_path);
        ~SQLiteDB();
        using SQLiteCallBack = int (*)(void *para, int colnum, char **colvalue, char **colname);
        // int open(const std::string &db_path);
        int exec(const char *sql, SQLiteDB::SQLiteCallBack cb, void *arg, char **err_msg);
        int get_tables(std::vector<std::string> &table_names);
        bool table_is_exist(const std::string &table_name);
        friend class SQLiteStmt;

    private:
        sqlite3 *_db;
    };

    //sqlite语句。执行时请确保SQLiteDB存在。
    class SQLiteStmt
    {
    public:
        SQLiteStmt(SQLiteDB &db, const std::string &sql);
        ~SQLiteStmt();
        int step(); //算出SQL语句结果的一行
        int reset();
        int column_count();           //返回当前行的列数。
        int column_type(int icol);    //返回指定列的类型
        int column_bytes(int icol);   //返回指定列值的字节数。
        int column_bytes16(int icol); //返回指定列值的字节数。utf16版本

        int column_int(int icol);                   //以int形式返回指定列的值。
        int64_t column_int64(int icol);             //以int64形式返回指定列的值
        double column_double(int icol);             //以double形式返回指定列的值
        const unsigned char *column_text(int icol); //以TEXT形式返回指定列的值，也就是一个字符串，编码格式utf8，长度用bytes获得
        const void *column_text16(int icol);        //以TEXT形式返回指定列的值，也就是一个字符串，编码格式utf16，长度用bytes16获得。
        const void *column_blob(int icol);          //以BLOB形式返回值，是一段二进制数据，长度使用bytes获得。

        void column_value(int icol);

        //bind相关接口
        int bind_blob(int idx, const void *data, int data_n, void (*del_func)(void *));
        int bind_blob64(int idx, const void *data, uint64_t n, void (*del_func)(void *));
        int bind_double(int idx, double val);
        int bind_int(int idx, int val);
        int bind_int64(int idx, int64_t val);
        int bind_null(int idx);
        int bind_text(int idx, const char *data, int data_n, void (*del_func)(void *));
        int bind_text16(int idx, const void *data, int data_n, void (*del_func)(void *));
        int bind_text64(int idx, const char *data, uint64_t data_n, void (*del_func)(void *), unsigned char encoding);
        int bind_value(int idx, const sqlite3_value *val);
        int bind_pointer(int idx, void *ptr, const char *type, void (*del_func)(void *));
        int bind_zeroblob(int idx, int n);
        int bind_zeroblob64(int idx, uint64_t n);

    private:
        sqlite3_stmt *_stmt;
    };
} // namespace ylib
