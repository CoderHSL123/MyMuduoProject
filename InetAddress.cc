#include "InetAddress.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
InetAddress::InetAddress(uint16_t port, std::string ip){
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    //inet_addr()函数将一个 字符串 形式的点分十进制的IP地址转换成一个网络字节序的整数
    inet_pton(AF_INET,ip.c_str(),(void*)&addr_.sin_addr.s_addr);
}

//inet_addr 将一个 点分十进制的IP地址 转换成一个 网络字节序 的整数 仅支持IPV4
//inet_pton 将一个 点分十进制的IP地址 转换成一个 网络字节序 的整数 支持IPV4和IPV6
//inet_ntop 将一个 网络字节序 的整数转换成一个 点分十进制的IP地址

//获取ip
std::string InetAddress::toIp() const{
    char buf[64] = {0};
    //将网络地址（二进制格式）转换为可读的字符串格式（例如 "192.168.1.1"）
    inet_ntop(AF_INET, &addr_.sin_addr,buf,sizeof(buf));
    return std::string(buf);
}
//获取port
uint16_t InetAddress::toPort() const{
    return ntohs(addr_.sin_port);
}
//获取ip和port
std::string InetAddress::toIpPort() const{
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr,buf,sizeof(buf));
    char buf2[128] = {0};
    sprintf(buf2,"%s:%u",buf,ntohs(addr_.sin_port));
    return std::string(buf2);
}
