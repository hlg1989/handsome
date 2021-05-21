//
// Created by gwe on 21-5-20.
//

#include "timer.h"

namespace handsome{

    atomic_int64_t timer::s_num_created;

    void timer::restart(timestamp now)
    {
        if(m_repeat){
            m_expiration = add_time(now, m_interval);
        }else{
            m_expiration = timestamp::invalid();
        }
    }
}