#ifndef NETWORKHANDLER_H
#define NETWORKHANDLER_H

/*
网络处理接口
抽象基类 
*/
#include <QByteArray>
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
    //关闭连接
    virtual void close() = 0;
    //写数据
    virtual qint64 write(const QByteArray &data) = 0;
    //查询连接状态
    virtual bool isConnected() const = 0;
    //查询模式
    virtual Mode mode() const = 0;

protected:
    QPair<QHostAddress, quint16> m_address;

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
