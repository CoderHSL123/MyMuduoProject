#pragma once
#include <netinet/in.h>
#include <string>

class InetAddress{
public:
    explicit InetAddress(uint16_t port, std::string ip="127.0.0.1");
    explicit InetAddress(const sockaddr_in& addr):addr_(addr) {
    }
    //获取ip
    std::string toIp() const;
    //获取port
    uint16_t toPort() const;
    //获取ip和port
    std::string toIpPort() const;
    const sockaddr_in * getSockAddr() const {
        return &addr_;
    }
private:
    sockaddr_in addr_;
};