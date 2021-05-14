//
// Created by gwe on 21-5-13.
//

#include "timer.h"

namespace handsome{

    void timer::restart(timestamp when)
    {
        if(m_repeat){
            m_expiration = add_time(when, m_interval);
        }else{
            m_expiration = timestamp::invalid();
        }
    }
}