//
// Created by gwe on 21-4-12.
//

#include "exception.h"
#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>

namespace handsome{

    exception::exception(const char *what)
        : m_message(what)
    {
        const int len = 200;
        void* buffer[len];
        int nptrs = ::backtrace(buffer, len);
        char** strings = ::backtrace_symbols(buffer, nptrs);
        if(strings){
            for(int i = 0; i < nptrs; ++i){
                m_stack.append(strings[i]);
                m_stack.push_back('\n');
            }

            free(strings);
        }
    }

    exception::~exception() throw()
    {

    }

    const char* exception::what() const throw()
    {
        return m_message.c_str();
    }

    const char* exception::stack_trace() const throw()
    {
        return m_stack.c_str();
    }
}
