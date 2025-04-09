#include "tcpclienthandler.h"

TCPClientHandler::TCPClientHandler(QObject *parent)
    : NetworkHandler{parent}
    , m_socket(QTcpSocket(this))
{
    connectSocketSignals();
}

bool TCPClientHandler::init(QPair<QHostAddress, quint16> address)
{
    m_socket.connectToHost(address.first, address.second);
    return isConnected();
}

void TCPClientHandler::close()
{
    //如果不是处于断开状态
    if (m_socket.state() != QAbstractSocket::UnconnectedState) {
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
                emit errorOccurred(m_socket.errorString());
            });
}

NetworkHandler::Mode TCPClientHandler::mode() const
{
    return Mode::TCP_CLIENT;
}
