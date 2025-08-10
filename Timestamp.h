#pragma once
#include <iostream>

class Timestamp{
public:
    //默认构造
    Timestamp();
    //带参构造
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    
    static Timestamp now();
    //获取当前时间的 年月日时分秒 输出
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};