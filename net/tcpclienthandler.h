#ifndef TCPCLIENTHANDLER_H
#define TCPCLIENTHANDLER_H

#include <QTcpSocket>
#include "networkhandler.h"

class TCPClientHandler : public NetworkHandler
{
    Q_OBJECT
public:
    explicit TCPClientHandler(QObject *parent);
    explicit TCPClientHandler(QObject *parent, qintptr sockfd);
    explicit TCPClientHandler(QObject *parent, QTcpSocket *client);
    bool init(QPair<QHostAddress, quint16> address) override;
    Mode mode() const override;

private:
    //连接qt原有信号
    void connectSocketSignals();

signals:
    //收到数据的时候同时传递套接字文件描述符（主要是给TCP Server用）
    void dataReceivedWithFd(qintptr socketFd, const QByteArray &data);
};

#endif // TCPCLIENTHANDLER_H
