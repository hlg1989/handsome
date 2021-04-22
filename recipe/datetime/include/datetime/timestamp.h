//
// Created by gwe on 21-4-16.
//

#ifndef MISC_TEST_TIMESTAMP_H
#define MISC_TEST_TIMESTAMP_H

#include "thread/noncopyable.h"
#include <stdint.h>
#include <string>

namespace handsome{

    class timestamp
    {
    public:
        timestamp();

        explicit timestamp(int64_t microseconds_since_epoch);

        void swap(timestamp& rhs)
        {
            std::swap(m_microseconds_since_spoch, rhs.m_microseconds_since_spoch);
        }

        std::string to_string() const;
        std::string to_formatted_string() const;

        bool vaild() const
        {
            return m_microseconds_since_spoch > 0;
        }

        int64_t  microseconds_since_epoch() const
        {
            return m_microseconds_since_spoch;
        }

        static timestamp now();
        static timestamp invalid();

        static const int kMicroSecondsPerSecond = 1000 * 1000;

    private:
        int64_t m_microseconds_since_spoch;
    };

    inline bool operator<(timestamp lhs, timestamp rhs)
    {
        return lhs.microseconds_since_epoch() < rhs.microseconds_since_epoch();
    }

    inline bool operator==(timestamp lhs, timestamp rhs)
    {
        return lhs.microseconds_since_epoch() == rhs.microseconds_since_epoch();
    }

    inline double time_difference(timestamp high, timestamp low)
    {
        int64_t diff = high.microseconds_since_epoch() - low.microseconds_since_epoch();
        return static_cast<double>(diff) / timestamp::kMicroSecondsPerSecond;
    }

    inline timestamp add_time(timestamp ts, double seconds)
    {
        int64_t  delta = static_cast<int64_t >(seconds * timestamp::kMicroSecondsPerSecond);
        return timestamp(ts.microseconds_since_epoch() + delta);
    }
}

#endif //MISC_TEST_TIMESTAMP_H
