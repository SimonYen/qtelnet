#ifndef GLOBALVARIABLE_HPP
#define GLOBALVARIABLE_HPP

#include <QString>

//套接字（连接）类型枚举
enum SocketType {
    TCP_CLIENT,
    TCP_SERVER,
    UDP,
};

//套接字类型转化为字符串输出
inline QString SocketTypeToString(int socketType)
{
    //转换成枚举
    SocketType st = static_cast<SocketType>(socketType);
    switch (st) {
    case SocketType::TCP_CLIENT:
        return "TCP客户端模式";
    case SocketType::TCP_SERVER:
        return "TCP服务器模式";
    case SocketType::UDP:
        return "UDP模式";
    default:
        return "未知";
    }
}

#endif // GLOBALVARIABLE_HPP
