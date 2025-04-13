#ifndef TCPCLIENTHANDLER_H
#define TCPCLIENTHANDLER_H

#include <QTcpSocket>
#include "networkhandler.h"

class TCPClientHandler : public NetworkHandler
{
    Q_OBJECT
public:
    explicit TCPClientHandler(QObject *parent = nullptr);
    bool init(QPair<QHostAddress, quint16> address) override;
    Mode mode() const override;

private:
    //连接qt原有信号
    void connectSocketSignals();
};

#endif // TCPCLIENTHANDLER_H
