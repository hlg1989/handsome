//
// Created by gwe on 21-4-13.
//

#ifndef MISC_TEST_SIGNAL_SLOT_H
#define MISC_TEST_SIGNAL_SLOT_H

#include "mutex.h"
#include "noncopyable.h"
#include <functional>
#include <memory>
#include <vector>

namespace handsome{
    namespace detail{
        template <typename Callback>
        struct slot_impl;

        template<typename Callback>
        struct signal_impl : private noncopyable
        {
        public:
            typedef std::vector<std::weak_ptr<slot_impl<Callback>>> slot_list;

            signal_impl()
                : m_slots(new slot_list())
            {

            }

            void copy_on_write()
            {
                m_mtx.assert_locked();
                if(!m_slots.unique()){
                    m_slots.reset(new slot_list(*m_slots));
                }
                assert(m_slots.unique());
            }

            void clean()
            {
                mutex_lock_guard lock(m_mtx);
                copy_on_write();
                slot_list& list(*m_slots);
                typename slot_list::iterator it = list.begin();
                while(it != list.end()){
                    if(it->expired()){
                        it = list.erase(it);
                    }else{
                        ++it;
                    }
                }
            }


            mutex m_mtx;
            std::shared_ptr<slot_list> m_slots;
        };

        template<typename Callback>
        struct slot_impl : private noncopyable
        {
        public:
            typedef signal_impl<Callback> Data;

            slot_impl(const std::shared_ptr<Data>& data, Callback&& cb)
                : m_data(data)
                , m_cb(cb)
                , m_tie()
                , m_tied(false)
            {

            }

            slot_impl(const std::shared_ptr<Data>& data, Callback&& cb,
                      const std::shared_ptr<void>tie)
                : m_data(data)
                , m_cb(cb)
                , m_tie(tie)
                , m_tied(true)
            {

            }

            ~slot_impl()
            {
                std::shared_ptr<Data> data(m_data.lock());
                if(data){
                    data->clean();
                }
            }

            std::weak_ptr<Data> m_data;
            Callback m_cb;
            std::weak_ptr<void> m_tie;
            bool m_tied;
        };
    }

    typedef std::shared_ptr<void>Slot;

    template <typename Signature>
    class signal;

    template<typename RET, typename... ARGS>
    class signal<RET(ARGS...)> : private noncopyable
    {
    public:
        typedef std::function<void(ARGS...)>Callback;
        typedef detail::signal_impl<Callback> signal_impl;
        typedef detail::slot_impl<Callback> slot_impl;

        signal()
            : m_impl(new signal_impl())
        {

        }

        ~signal()
        {

        }

        Slot connect(Callback&& func)
        {
            std::shared_ptr<slot_impl> slot(new slot_impl(m_impl, std::forward(func)));
            add(slot);
            return slot;
        }

        Slot connect(Callback&& func, const std::shared_ptr<void>& tie)
        {
            std::shared_ptr<slot_impl> slot(new slot_impl(m_impl, std::forward(func), tie));
            add(slot);
            return slot;
        }

        void call(ARGS&&... args)
        {
            signal_impl& impl(*m_impl);
            std::shared_ptr<typename signal_impl::slot_list> slots;
            {
                mutex_lock_guard lock(impl.m_mtx);
                slots = impl.m_slots;
            }

            typename signal_impl::slot_list& s(*slots);
            for(typename signal_impl::slot_list::const_iterator it = s.cbegin(); it != s.cend(); ++it){
                std::shared_ptr<slot_impl>slot = it->lock();
                if(slot){
                    std::shared_ptr<void>guard;
                    if(slot->m_tie){
                        guard = slot->m_tie.lock();
                        if(guard){
                            slot->m_cb(std::forward(args)...);
                        }
                    }else{
                        slot->m_cb(std::forward(args)...);
                    }
                }
            }
        }

    private:
        void add(const std::shared_ptr<slot_impl>& slot)
        {
            signal_impl& impl(*m_impl);
            {
                mutex_lock_guard lock(impl.m_mtx);
                impl.copy_on_write();
                impl.m_slots->push_back(slot);
            }
        }

    private:
        const std::shared_ptr<signal_impl> m_impl;
    };
}


#endif //MISC_TEST_SIGNAL_SLOT_H
