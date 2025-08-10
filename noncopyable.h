#pragma once


/*
    子类继承该类以后，子类对象不能调用拷贝构造与赋值构造
*/
class Noncopyable{
public:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() = default;

};
