//
// Created by gwe on 21-5-20.
//

#ifndef HANDSOME_TIMER_ID_H
#define HANDSOME_TIMER_ID_H

namespace handsome{

    class timer;

    class timer_id
    {
    public:

        timer_id(timer* tm = nullptr, int64_t seq = 0)
            : m_timer(tm)
            , m_sequeue(seq)
        {

        }

        friend class timer_queue;
    private:

        timer* m_timer;
        int64_t m_sequeue;
    };
}

#endif //HANDSOME_TIMER_ID_H
