#include "tcpclienthandler.h"
#include <QDebug>

TCPClientHandler::TCPClientHandler(QObject *parent)
    : NetworkHandler{parent}
    , m_socket(QTcpSocket(this))
{
    connectSocketSignals();
}

bool TCPClientHandler::init(QPair<QHostAddress, quint16> address)
{
    m_address = address;
    m_socket.connectToHost(address.first, address.second);
    //不能立马查看连接状态的
    if (m_socket.waitForConnected()) {
        qInfo() << address << " connected.";
        return true;
    } else {
        return false;
    }
}

void TCPClientHandler::close()
{
    //如果不是处于断开状态
    if (m_socket.state() != QAbstractSocket::UnconnectedState) {
        qInfo() << m_address << " disconnected.";
        //断开
        m_socket.disconnectFromHost();
    }
}

qint64 TCPClientHandler::write(const QByteArray &data)
{
    return m_socket.write(data);
}

bool TCPClientHandler::isConnected() const
{
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

void TCPClientHandler::connectSocketSignals()
{
    connect(&m_socket, &QTcpSocket::connected, this, &NetworkHandler::connected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &NetworkHandler::disconnected);
    connect(&m_socket, &QTcpSocket::readyRead, [this]() { emit dataReceived(m_socket.readAll()); });
    connect(&m_socket,
            QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            [this](QAbstractSocket::SocketError error) {
                //顺便也直接日志记录
                qCritical() << m_socket.errorString();
                emit errorOccurred(m_socket.errorString());
            });
}

NetworkHandler::Mode TCPClientHandler::mode() const
{
    return Mode::TCP_CLIENT;
}
