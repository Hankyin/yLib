#pragma once
#include <string>
#include <sys/time.h>

namespace ylib
{
    class Date
    {
    public:
        Date();
        Date(int y, int m, int d);
        ~Date();
        //计算年月日的增长好麻烦啊
        Date add_days(uint64_t ndays) const;
        Date add_months(int nmonths) const;
        Date add_years(int nyears) const;
        int day() const;
        int month() const;
        int year() const;

        std::string to_string() const;

    private:
        int _day = 0;
        int _month = 0;
        int _year = 0;
    };

    class Time
    {
    public:
        Time();
        Time(int h, int m, int s, int ms = 0);
        ~Time();
        Time add_hour(int h) const;
        Time add_minite(int m) const;
        Time add_second(int s) const;
        Time add_ms(int ms) const;

        int hour() const;
        int minute() const;
        int second() const;
        int msecond() const;

        int msec_since_start_of_day() const;
        timeval to_timeval() const;

        std::string to_string() const;

    private:
        int _h;
        int _m;
        int _s;
        int _ms;
    };

    class DateTime
    {
    public:
        DateTime();
        DateTime(int timestamp);
        DateTime(const Date &date, const Time &time);
        ~DateTime();
        std::string to_string();

        int get_timestamp();
        long get_timestamp_ms(); //获取ms时间戳
        Date date();
        Time time();

        static DateTime now();

        bool operator==(const DateTime &);
        bool operator!=(const DateTime &);
        bool operator<(const DateTime &);
        bool operator>(const DateTime &);

    private:
        timeval _tv;
    };

} // namespace ylib
