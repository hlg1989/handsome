//
// Created by gwe on 21-5-13.
//

#ifndef HANDSOME_TIMER_ID_H
#define HANDSOME_TIMER_ID_H

namespace handsome{

    class timer;

    class timer_id
    {
    public:

        explicit timer_id(timer* tm)
            : m_timer(tm)
        {

        }

    private:

        timer* m_timer;
    };
}


#endif //HANDSOME_TIMER_ID_H
