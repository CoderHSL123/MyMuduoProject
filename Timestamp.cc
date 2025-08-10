
#include "Timestamp.h"
#include <ctime>
//默认构造
Timestamp::Timestamp():microSecondsSinceEpoch_(0){

}
//带参构造
Timestamp::Timestamp(int64_t microSecondsSinceEpoch):microSecondsSinceEpoch_(microSecondsSinceEpoch){

}
    
Timestamp Timestamp::now(){
    //获取当前时间戳
    time_t ti  = time(NULL);
    return Timestamp(ti);
}
//获取当前时间的 年月日时分秒 输出
std::string Timestamp::toString() const{
    char buf[128];
    struct tm * tim = localtime(&microSecondsSinceEpoch_);
    //%4d 年  %02d 月  %02d 日  %02d 时  %02d 分  %02d 秒
    snprintf(buf, 128,"%4d/%02d/%02d %02d:%02d:%02d", 
        tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, 
        tim->tm_hour, tim->tm_min, tim->tm_sec);

    return buf;
}
