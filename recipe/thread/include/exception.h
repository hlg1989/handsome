//
// Created by gwe on 21-4-12.
//

#ifndef MISC_TEST_EXCEPTION_H
#define MISC_TEST_EXCEPTION_H

#include <exception>
#include <string>

namespace handsome{

    class exception : public std::exception
    {
    public:
        explicit exception(const char* what);
        virtual ~exception() throw();
        virtual const char* what() const throw();
        const char* stack_trace() const throw();

    private:
        std::string m_message;
        std::string m_stack;
    };
}

#endif //MISC_TEST_EXCEPTION_H
