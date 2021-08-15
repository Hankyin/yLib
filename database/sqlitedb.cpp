#include "sqlitedb.h"
#include "core/logger.h"
#include <exception>
#include <string>
#include <stdexcept>

namespace ylib
{

    //
    //SQLiteStmt 执行
    //
    SQLiteStmt::SQLiteStmt(SQLiteDB &db, const std::string &sql)
    {
        int r = sqlite3_prepare_v3(db._db, sql.c_str(), sql.size(), 0, &_stmt, NULL);
        if (r != SQLITE_OK)
        {
            throw SQLiteStmtException(r);
        }
    }

    SQLiteStmt::~SQLiteStmt()
    {
        sqlite3_finalize(_stmt);
    }

    int SQLiteStmt::step()
    {
        int r = sqlite3_step(_stmt);
        if (r == SQLITE_OK || r == SQLITE_ROW || r == SQLITE_DONE)
        {
            return r;
        }
        else
        {
            throw SQLiteStepException(r);
        }
    }

    int SQLiteStmt::reset()
    {
        return sqlite3_reset(_stmt);
    }

    int SQLiteStmt::column_count()
    {
        return sqlite3_column_count(_stmt);
    }

    int SQLiteStmt::column_bytes(int icol)
    {
        return sqlite3_column_bytes(_stmt, icol);
    }

    int SQLiteStmt::column_bytes16(int icol)
    {
        return sqlite3_column_bytes16(_stmt, icol);
    }

    int SQLiteStmt::column_int(int icol)
    {
        return sqlite3_column_int(_stmt, icol);
    }

    int64_t SQLiteStmt::column_int64(int icol)
    {
        return sqlite3_column_int64(_stmt, icol);
    }

    double SQLiteStmt::column_double(int icol)
    {
        return sqlite3_column_double(_stmt, icol);
    }

    const unsigned char *SQLiteStmt::column_text(int icol)
    {
        return sqlite3_column_text(_stmt, icol);
    }

    const void *SQLiteStmt::column_text16(int icol)
    {
        return sqlite3_column_text16(_stmt, icol);
    }

    const void *SQLiteStmt::column_blob(int icol)
    {
        return sqlite3_column_blob(_stmt, icol);
    }

    int SQLiteStmt::bind_blob(int idx, const void *data, int data_n, void (*del_func)(void *))
    {
        return sqlite3_bind_blob(_stmt, idx, data, data_n, del_func);
    }

    int SQLiteStmt::bind_blob64(int idx, const void *data, uint64_t n, void (*del_func)(void *))
    {
        return sqlite3_bind_blob64(_stmt, idx, data, n, del_func);
    }

    int SQLiteStmt::bind_double(int idx, double val)
    {
        return sqlite3_bind_double(_stmt, idx, val);
    }

    int SQLiteStmt::bind_int(int idx, int val)
    {
        return sqlite3_bind_int(_stmt, idx, val);
    }

    int SQLiteStmt::bind_int64(int idx, int64_t val)
    {
        return sqlite3_bind_int64(_stmt, idx, val);
    }

    int SQLiteStmt::bind_null(int idx)
    {
        return sqlite3_bind_null(_stmt, idx);
    }

    int SQLiteStmt::bind_text(int idx, const char *data, int data_n, void (*del_func)(void *))
    {
        return sqlite3_bind_text(_stmt, idx, data, data_n, del_func);
    }

    int SQLiteStmt::bind_text16(int idx, const void *data, int data_n, void (*del_func)(void *))
    {
        return sqlite3_bind_text16(_stmt, idx, data, data_n, del_func);
    }

    int SQLiteStmt::bind_text64(int idx, const char *data, uint64_t data_n, void (*del_func)(void *), unsigned char encoding)
    {
        return sqlite3_bind_text64(_stmt, idx, data, data_n, del_func, encoding);
    }

    int SQLiteStmt::bind_value(int idx, const sqlite3_value *val)
    {
        return sqlite3_bind_value(_stmt, idx, val);
    }

    int SQLiteStmt::bind_pointer(int idx, void *ptr, const char *type, void (*del_func)(void *))
    {

        return sqlite3_bind_pointer(_stmt, idx, ptr, type, del_func);
    }

    int SQLiteStmt::bind_zeroblob(int idx, int n)
    {
        return sqlite3_bind_zeroblob(_stmt, idx, n);
    }

    int SQLiteStmt::bind_zeroblob64(int idx, uint64_t n)
    {
        return sqlite3_bind_zeroblob64(_stmt, idx, n);
    }

    //
    // 数据库连接对象
    //
    SQLiteDB::SQLiteDB(const std::string &db_path)
    {
        int res = sqlite3_open(db_path.c_str(), &_db);
        if (res == SQLITE_OK)
        {
            LOG_INFO << "sqlite database connected [" << db_path << "]";
        }
        else
        {
            LOG_ERROR << "sqlite database connect fail [" << res << "]";
            throw SQLiteConnException(res);
        }
    }

    int SQLiteDB::get_tables(std::vector<std::string> &table_names)
    {
        std::string get_table_sql = "SELECT name FROM sqlite_master WHERE type='table'";
        SQLiteStmt stmt(*this, get_table_sql);
        while (true)
        {
            int r = stmt.step();
            if (r == SQLITE_ROW)
            {
                const char *name = (char *)stmt.column_text(0);
                if (name)
                {
                    std::string str = name;
                    table_names.push_back(str);
                }
                else
                {
                    table_names.push_back("");
                }
            }
            else if (r == SQLITE_DONE)
            {
                return 0;
            }
            else if (r == SQLITE_BUSY)
            {
                continue;
            }
            else
            {
                return -1;
            }
        }

        return 0;
    }

    bool SQLiteDB::table_is_exist(const std::string &table_name)
    {
        std::string sql = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='" + table_name + "'";
        SQLiteStmt stmt(*this, sql);
        while (true)
        {
            int r = stmt.step();
            if (r == SQLITE_ROW)
            {
                int c = stmt.column_int(0);
                if (c == 1)
                {
                    return true;
                }
            }
            else
            {
                break;
            }
        }
        return false;
    }

    int SQLiteDB::exec(const char *sql, SQLiteDB::SQLiteCallBack cb, void *arg, char **err_msg)
    {
        return sqlite3_exec(_db, sql, cb, arg, err_msg);
    }

    SQLiteDB::~SQLiteDB()
    {
        sqlite3_close(_db);
    }

} // namespace ylib
