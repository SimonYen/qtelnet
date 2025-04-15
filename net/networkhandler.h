#ifndef NETWORKHANDLER_H
#define NETWORKHANDLER_H

/*
网络处理接口
抽象基类 
*/
#include <QByteArray>
#include <QDebug>
#include <QHostAddress>
#include <QObject>
#include <QPair>
#include <QString>

class NetworkHandler : public QObject
{
    Q_OBJECT
public:
    enum class Mode { TCP_CLIENT, TCP_SERVER, UDP };

    //模式转化为字符串输出
    static QString mode2String(int mode)
    {
        //转化为枚举
        Mode m = static_cast<Mode>(mode);
        switch (m) {
        case Mode::TCP_CLIENT:
            return "TCP客户端模式";
        case Mode::TCP_SERVER:
            return "TCP服务器模式";
        case Mode::UDP:
            return "UDP模式";
        default:
            return "未知模式";
        }
    }

    explicit NetworkHandler(QObject *parent = nullptr)
        : QObject(parent)
    {}

    //初始化连接
    virtual bool init(QPair<QHostAddress, quint16>) = 0;
    //查询模式
    virtual Mode mode() const = 0;

    /*非虚函数，基类就可以实现的，逻辑通用*/

    //关闭连接
    virtual void close()
    {
        //未初始化，直接跳过
        if (m_socket == nullptr)
            return;
        //如果不是处于断开状态
        if ((m_socket->state()) != QAbstractSocket::UnconnectedState) {
            qInfo() << m_address << " disconnected.";
            //断开
            m_socket->disconnectFromHost();
        }
    }
    //写数据
    qint64 write(const QByteArray &data) { return m_socket->write(data); }
    //一次性写完所有数据
    bool writeAll(const QByteArray &data)
    {
        //当前写入字节数
        qint64 totalWritten = 0;
        //总共字节数
        qint64 toWrite = data.size();
        //循环写入
        while (totalWritten < toWrite) {
            qint64 written = m_socket->write(data.constData() + totalWritten,
                                             toWrite - totalWritten);
            if (written == -1) {
                qWarning() << "Writing failed: " << m_socket->errorString();
                return false;
            }
            //更新写入字节数
            totalWritten += written;
            //最多等待三秒写完
            if (!(m_socket->waitForBytesWritten(3000))) {
                qWarning() << "Writing times out: " << m_socket->errorString();
                return false;
            }
        }
        return true;
    }
    //查询连接状态
    bool isConnected() const { return (m_socket->state()) == QAbstractSocket::ConnectedState; }
    //获取本地地址
    virtual QString localAddress() const { return m_socket->localAddress().toString(); }
    //获取本地端口
    virtual quint16 localPort() const { return m_socket->localPort(); }
    //获取对方地址
    QString peerAddress() const { return m_socket->peerAddress().toString(); }
    //获取对方端口
    quint16 peerPort() const { return m_socket->peerPort(); }

protected:
    QPair<QHostAddress, quint16> m_address;
    QAbstractSocket *m_socket;

    //信号
signals:
    //连接成功信号
    void connected();
    //连接断开信号
    void disconnected();
    //收到数据信号
    void dataReceived(const QByteArray &data);
    //发生错误信号
    void errorOccurred(const QString &error);
};

#endif // NETWORKHANDLER_H
