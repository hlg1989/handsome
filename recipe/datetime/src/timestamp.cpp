//
// Created by gwe on 21-4-16.
//

#include "datetime/timestamp.h"
#include <stdio.h>
#include <sys/time.h>
#define __STDC_FORMAT_MACROS

#include <inttypes.h>

#undef __STDC_FORMAT_MACROS

namespace handsome{

    static_assert(sizeof(timestamp) == sizeof(int64_t));

    timestamp::timestamp()
        : m_microseconds_since_spoch(0)
    {

    }

    timestamp::timestamp(int64_t microseconds_since_epoch)
        : m_microseconds_since_spoch(microseconds_since_epoch)
    {

    }

    std::string timestamp::to_string() const
    {
        char buf[32] = {0};
        int64_t seconds = m_microseconds_since_spoch / kMicroSecondsPerSecond;
        int64_t microseconds = m_microseconds_since_spoch % kMicroSecondsPerSecond;

        snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
        return buf;
    }

    std::string timestamp::to_formatted_string() const
    {
        char buf[64] = {0};
        time_t seconds = static_cast<time_t>(m_microseconds_since_spoch / kMicroSecondsPerSecond);
        int microseconds = static_cast<int>(m_microseconds_since_spoch % kMicroSecondsPerSecond);

        struct tm tm_time;
        gmtime_r(&seconds, &tm_time);

        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);

        return buf;
    }

    timestamp timestamp::now()
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t seconds = tv.tv_sec;
        return timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
    }

    timestamp timestamp::invalid()
    {
        return timestamp();
    }
}