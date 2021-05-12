//
// Created by gwe on 21-5-8.
//

#include "timer.h"

namespace handsome{

    void timer::restart(timestamp now)
    {
        if(m_repeat){
            m_expiration = add_time(now, m_interval);
        }else{
            m_expiration = timestamp::invalid();
        }
    }
}