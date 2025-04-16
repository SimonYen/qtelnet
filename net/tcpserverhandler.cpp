#include "tcpserverhandler.h"
#include "../utils.h"

#include <QDebug>
#include <QThread>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

TCPServerHandler::TCPServerHandler(QObject *parent)
    : NetworkHandler{parent}
{
    m_server = new QTcpServer(parent);
    m_socket = nullptr;
}

TCPServerHandler::~TCPServerHandler()
{
    if (m_server) {
        delete m_server;
        m_server = nullptr;
    }
    //Windows下还需要额外处理
#ifdef Q_OS_WIN
    WSACleanup();
#endif
}

bool TCPServerHandler::start(quint16 port)
{
    // 1. 创建 socket
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        qCritical() << "Failed to create socket";
        return false;
    }

    // 2. 设置端口复用
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &opt, sizeof(opt)) < 0) {
        qCritical() << "setsockopt(SO_REUSEADDR) failed";
        common_close_fd(sockfd);
        return false;
    }

    // 3. 绑定地址
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(sockfd, (sockaddr *) &addr, sizeof(addr)) < 0) {
        qCritical() << "bind() failed";
        common_close_fd(sockfd);
        return false;
    }

    // 4. 设置为监听
    if (::listen(sockfd, SOMAXCONN) < 0) {
        qCritical() << "listen() failed";
        common_close_fd(sockfd);
        return false;
    }

    // 5. 设置 socket 到 QTcpServer
    if (!(m_server->setSocketDescriptor(sockfd))) {
        qCritical() << "setSocketDescriptor() failed";
        common_close_fd(sockfd);
        return false;
    }

    qInfo() << "Server with SO_REUSEADDR listening on port" << port;
    return true;
}

bool TCPServerHandler::sendToClientByFd(qintptr fd, const QByteArray &data)
{
    //查询客户端
    if (!m_fd2client.contains(fd)) {
        qWarning() << "Fd: " << fd << " doesn't exisit!";
        return false;
    }
    //存在的话，直接写
    return m_fd2client[fd]->writeAll(data);
}

bool TCPServerHandler::sendToClientByAddress(QPair<QHostAddress, quint16> addr,
                                             const QByteArray &data)
{
    //查询客户端
    if (!m_address2fd.contains(addr)) {
        qWarning() << "Address: " << addr << " doesn't exisit!";
        return false;
    }
    return sendToClientByFd(m_address2fd[addr], data);
}

bool TCPServerHandler::sendToAllClient(const QByteArray &data)
{
    bool result = true;
    //遍历所有客户端
    for (auto &fd : m_fd2client.keys()) {
        if (!sendToClientByFd(fd, data)) {
            result = false;
        }
    }
    return result;
}

bool TCPServerHandler::init(QPair<QHostAddress, quint16> address)
{
    //连接信号和槽
    connect(m_server, &QTcpServer::newConnection, [&]() {
        //处理新连接
        while (m_server->hasPendingConnections()) {
            auto client = m_server->nextPendingConnection();
            incomingConnection(client);
        }
    });
    //Windows平台下还需要加载额外的东西
#ifdef Q_OS_WIN
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        qCritical() << "WSAStartup failed";
        return false;
    }
#endif
    return start(address.second);
}

QString TCPServerHandler::localAddress() const
{
    return m_server->serverAddress().toString();
}

quint16 TCPServerHandler::localPort() const
{
    return m_server->serverPort();
}

void TCPServerHandler::close()
{
    if (m_server) {
        qInfo() << "Staring to close self...";
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }
}

void TCPServerHandler::shutdown()
{
    for (auto &fd : m_fd2client.keys()) {
        qInfo() << "Fd: " << fd << " - closing connection and stopping thread...";

        auto client = m_fd2client[fd];
        auto thread = m_fd2thread[fd];

        if (client) {
            // 使用 deleteLater + disconnect
            client->disconnect(); // 防止发出信号
            client->close();
            client->deleteLater();
        }

        if (thread) {
            thread->quit();
            if (!thread->wait(3000)) {
                qWarning() << "Thread did not quit in time. Forcing termination.";
                thread->terminate(); // 或者强制终止，但要注意安全性
                thread->wait();
            }
            thread->deleteLater();
        }
    }

    m_fd2client.clear();
    m_fd2address.clear();
    m_fd2thread.clear();
    m_address2fd.clear();
}

NetworkHandler::Mode TCPServerHandler::mode() const
{
    return NetworkHandler::Mode::TCP_SERVER;
}

void TCPServerHandler::common_close_fd(int sockfd)
{
#ifdef Q_OS_WIN
    closesocket(sockfd);
#else
    close(sockfd);
#endif
}

void TCPServerHandler::incomingConnection(QTcpSocket *client)
{
    //初始化客户端
    auto cli = new TCPClientHandler(this, client);
    //创建一个线程
    auto thread = new QThread(this);
    //这里还需要解除parent关系
    cli->setParent(nullptr);
    //移动到线程里
    cli->moveToThread(thread);
    //获取地址
    QPair<QHostAddress, quint16> clientAddress = {QHostAddress(cli->peerAddress()), cli->peerPort()};
    //获取套接字描述符
    auto fd = client->socketDescriptor();
    //线程开始运行时，先更新UI
    connect(thread, &QThread::started, this, [clientAddress, fd, this]() {
        emit clientComing(clientAddress);
        qInfo() << clientAddress << " is comming, Fd: " << fd;
    });
    //当客户端传来数据时
    connect(cli,
            &TCPClientHandler::dataReceivedWithFd,
            this,
            [this](qintptr socketfd, const QByteArray &data) {
                //获取地址
                auto addr = m_fd2address[socketfd];
                //转发给UI
                emit clientMessageSended(addr, data);
                qInfo() << addr << " says: " << ByteArrayUtils::toUtf8String(data);
            });
    //当客户端断开连接时，首先让线程退出
    connect(cli, &NetworkHandler::disconnected, this, [thread]() {
        thread->quit();
        //必须等待
        thread->wait();
    });
    //线程退出时，清理资源
    connect(thread, &QThread::finished, this, [thread, cli, fd, this]() {
        thread->deleteLater();
        cli->deleteLater();
        //获取地址
        auto addr = m_fd2address[fd];
        //UI变化
        emit clientLeaving(addr);
        qInfo() << addr << " disconnected positively.";
        m_address2fd.remove(addr);
        m_fd2address.remove(fd);
        m_fd2client.remove(fd);
        m_fd2thread.remove(fd);
    });
    //启动线程
    thread->start();
    m_address2fd[clientAddress] = fd;
    m_fd2address[fd] = clientAddress;
    m_fd2client[fd] = cli;
    m_fd2thread[fd] = thread;
}
