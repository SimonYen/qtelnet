#include "tcpclienthandler.h"
#include <QDebug>

TCPClientHandler::TCPClientHandler(QObject *parent)
    : NetworkHandler{parent}
{
    m_socket = new QTcpSocket(this);
    connectSocketSignals();
}

TCPClientHandler::TCPClientHandler(QObject *parent, qintptr sockfd)
    : NetworkHandler{parent}
{
    m_socket = new QTcpSocket();
    m_socket->setSocketDescriptor(sockfd);
    connectSocketSignals();
}

TCPClientHandler::TCPClientHandler(QObject *parent, QTcpSocket *client)
    : NetworkHandler{parent}
{
    m_socket = client;
    connectSocketSignals();
}

bool TCPClientHandler::init(QPair<QHostAddress, quint16> address)
{
    m_address = address;
    m_socket->connectToHost(address.first, address.second);
    //不能立马查看连接状态的
    if (m_socket->waitForConnected(1500)) {
        qInfo() << address << " connected.";
        return true;
    } else {
        return false;
    }
}

void TCPClientHandler::connectSocketSignals()
{
    connect(m_socket, &QTcpSocket::connected, this, &NetworkHandler::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkHandler::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, [this]() {
        emit dataReceived(m_socket->readAll());
        emit dataReceivedWithFd(m_socket->socketDescriptor(), m_socket->readAll());
    });
    connect(m_socket,
            QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            [this](QAbstractSocket::SocketError error) {
                //顺便也直接日志记录
                qCritical() << m_socket->errorString();
                emit errorOccurred(m_socket->errorString());
            });
}

NetworkHandler::Mode TCPClientHandler::mode() const
{
    return Mode::TCP_CLIENT;
}
