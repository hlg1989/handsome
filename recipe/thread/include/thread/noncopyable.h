//
// Created by gwe on 21-4-8.
//

#ifndef MISC_TEST_NONCOPYABLE_H
#define MISC_TEST_NONCOPYABLE_H


class noncopyable{
protected:
    noncopyable() = default;
    ~noncopyable() = default;

private:
    noncopyable(const noncopyable&) = delete;
    const noncopyable&operator=(const noncopyable&) = delete;
};
#endif //MISC_TEST_NONCOPYABLE_H
