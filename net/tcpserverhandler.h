#ifndef TCPSERVERHANDLER_H
#define TCPSERVERHANDLER_H

#include <QHash>
#include <QList>
#include <QTcpServer>
#include "networkhandler.h"
#include "tcpclienthandler.h"

class TCPServerHandler : public NetworkHandler
{
    Q_OBJECT
public:
    explicit TCPServerHandler(QObject *parent = nullptr);
    ~TCPServerHandler();
    //服务器开始运行
    bool start(quint16 port);
    //根据套接字描述符，给某个客户端发送数据
    bool sendToClientByFd(qintptr fd, const QByteArray &data);
    //根据地址，给某个客户端发送数据
    bool sendToClientByAddress(QPair<QHostAddress, quint16> addr, const QByteArray &data);
    bool init(QPair<QHostAddress, quint16> address) override;
    Mode mode() const override;
    QString localAddress() const override;
    quint16 localPort() const override;
    void close() override;

private:
    //跨平台关闭套接字
    void common_close_fd(int sockfd);
    //当有新连接到来时
    void incomingConnection(QTcpSocket *client);

private:
    //地址到套接字文件描述符的映射
    QHash<QPair<QHostAddress, quint16>, qintptr> m_address2fd;
    //套接字文件描述符到地址的映射
    QHash<qintptr, QPair<QHostAddress, quint16>> m_fd2address;
    //套接字文件描述符到客户端的映射
    QHash<qintptr, TCPClientHandler *> m_fd2client;
    //服务器句柄
    QTcpServer *m_server;
    QHash<qintptr, QThread *> m_fd2thread;

    //自定义信号
signals:
    //客户端消息发来
    void clientMessageSended(QPair<QHostAddress, quint16> address, const QByteArray &data);
    //客户端到来
    void clientComing(QPair<QHostAddress, quint16> address);
    //客户端离开
    void clientLeaving(QPair<QHostAddress, quint16> address);
};

#endif // TCPSERVERHANDLER_H
