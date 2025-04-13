#ifndef UTILS_H
#define UTILS_H

/*
通用工具的集合 
*/
#include <QByteArray>
#include <QDateTime>
#include <QString>

//字节转换类
class ByteArrayUtils
{
public:
    //转化为UTF8字符串
    static QString toUtf8String(const QByteArray &data);
    //转化为ASCII字符串
    static QString toAsciiString(const QByteArray &data);
    //转化为十六进制字符串
    static QString toHexString(const QByteArray &data, bool uppercase = false, bool spaced = false);
};

//消息记录构造类
class MessageBuilderUtils
{
private:
    QString m_msg;       //消息
    QString m_buildTime; //构建时间
    QString m_role;      //角色
    QString m_host;      //地址
    quint16 m_port;      //端口

public:
    MessageBuilderUtils(const QString &role, const QString &host, quint16 port);
    //获取朴素字符串
    QString toPlainText(const QString &msg);
    //获取HTML格式字符串
    QString toHTMLText(const QString &msg, const QString &color);
};

#endif // UTILS_H
