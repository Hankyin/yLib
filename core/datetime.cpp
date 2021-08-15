#include "datetime.h"
#include <time.h>
#include <sstream>

namespace ylib
{
    ///////////////////////////////////////////////////////////////////
    /////////////              DATE
    ///////////////////////////////////////////////////////////////////
    Date::Date()
    {
    }

    Date::Date(int y, int m, int d)
    {
        _year = y;
        _month = m;
        _day = d;
    }

    Date::~Date()
    {
    }

    Date Date::add_days(uint64_t ndays) const
    {
        Date new_data;
        int add_y = 0;
        int add_m = 0;
        int add_d = 0;
        new_data._day = _day + add_d;
        new_data._month = _month + add_m;
        new_data._year = _year + add_y;
        return new_data;
    }

    Date Date::add_months(int nmonths) const
    {
        Date new_data;
        int add_y = nmonths % 12;
        int add_m = nmonths / 12;
        new_data._day = _day;
        new_data._month = _month + add_m;
        new_data._year = _year + add_y;
        return new_data;
    }

    Date Date::add_years(int nyears) const
    {
        Date new_data;
        new_data._day = _day;
        new_data._year = _year;
        new_data._month = _month;
        return new_data;
    }

    int Date::day() const
    {
        return _day;
    }

    int Date::month() const
    {
        return _month;
    }
    int Date::year() const
    {
        return _year;
    }

    std::string Date::to_string() const
    {
        //2020-12-31
        std::stringstream fmt;
        fmt << _year << '-' << _month << '-' << _day;
        return fmt.str();
    }

    ///////////////////////////////////////////////////////////////////
    /////////////              TIME
    ///////////////////////////////////////////////////////////////////
    Time::Time()
    {
    }

    Time::Time(int h, int m, int s, int ms)
    {
        _h = h, _m = m, _s = s, _ms = ms;
    }

    Time::~Time()
    {
    }

    Time Time::add_hour(int h) const
    {
        Time new_time;
        return new_time;
    }

    Time Time::add_minite(int m) const
    {
        Time new_time;
        return new_time;
    }

    Time Time::add_second(int s) const
    {
        Time new_time;
        return new_time;
    }

    Time Time::add_ms(int ms) const
    {
        Time new_time;
        return new_time;
    }

    int Time::hour() const
    {
        return _h;
    }

    int Time::minute() const
    {
        return _m;
    }

    int Time::second() const
    {
        return _s;
    }

    int Time::msecond() const
    {
        return _ms;
    }

    int Time::msec_since_start_of_day() const
    {
        return (_h * 3600 + _m * 60 + _s) * 1000 + _ms;
    }

    std::string Time::to_string() const
    {
        std::stringstream fmt;
        fmt << _h << ':' << _m << ':' << _s << ' ' << _ms;
        return fmt.str();
    }

    timeval Time::to_timeval() const
    {
        struct tm tt;
        struct timeval tv;
        tt.tm_hour = _h;
        tt.tm_min = _m;
        tt.tm_sec = _s;
        tv.tv_sec = ::mktime(&tt);
        tv.tv_usec = _ms * 1000;
        return tv;
    }
    ///////////////////////////////////////////////////////////////////
    /////////////              DATETIME
    ///////////////////////////////////////////////////////////////////
    DateTime::DateTime()
    {
    }

    DateTime::DateTime(int timestamp)
    {
        _tv.tv_sec = timestamp;
        _tv.tv_usec = 0;
    }

    DateTime::DateTime(const Date &date, const Time &time)
    {
    }

    DateTime::~DateTime()
    {
    }

    std::string DateTime::to_string()
    {
        char buffer[40] = {0};
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S:", localtime(&_tv.tv_sec));
        return buffer + std::to_string(_tv.tv_usec);
    }

    DateTime DateTime::now()
    {
        DateTime dt;
        ::gettimeofday(&dt._tv, NULL);
        return dt;
    }

    int DateTime::get_timestamp()
    {
        return _tv.tv_sec;
    }

    long DateTime::get_timestamp_ms()
    {
        return _tv.tv_sec * 1000 + _tv.tv_usec / 1000;
    }

    Date DateTime::date()
    {
        tm *tt = localtime(&_tv.tv_sec);
        Date dt(tt->tm_year, tt->tm_mon, tt->tm_mday);
        return dt;
    }

    Time DateTime::time()
    {
        tm *tt = localtime(&_tv.tv_sec);
        Time tim(tt->tm_hour, tt->tm_min, tt->tm_sec, _tv.tv_usec / 1000);
        return tim;
    }

    bool DateTime::operator==(const DateTime &p)
    {
        return (_tv.tv_sec == p._tv.tv_sec) && (_tv.tv_usec == p._tv.tv_usec);
    }

    bool DateTime::operator!=(const DateTime &p)
    {
        return (_tv.tv_sec != p._tv.tv_sec) || (_tv.tv_usec != p._tv.tv_usec);
    }

    bool DateTime::operator<(const DateTime &p)
    {
        if (_tv.tv_sec == p._tv.tv_sec)
        {
            return _tv.tv_usec < p._tv.tv_usec;
        }
        else
        {
            return _tv.tv_sec < p._tv.tv_sec;
        }
    }

    bool DateTime::operator>(const DateTime &p)
    {
        if (_tv.tv_sec == p._tv.tv_sec)
        {
            return _tv.tv_usec > p._tv.tv_usec;
        }
        else
        {
            return _tv.tv_sec > p._tv.tv_sec;
        }
    }

} // namespace ylib
