#include "utils.h"

QString ByteArrayUtils::toUtf8String(const QByteArray &data)
{
    return QString::fromUtf8(data);
}

QString ByteArrayUtils::toAsciiString(const QByteArray &data)
{
    return QString::fromLatin1(data);
}

QString ByteArrayUtils::toHexString(const QByteArray &data, bool uppercase, bool spaced)
{
    if (!spaced) {
        QString hex = data.toHex();
        return uppercase ? hex.toUpper() : hex;
    } else {
        QString result;
        for (auto byte : data) {
            result += QString("%1 ").arg(static_cast<quint8>(byte), 2, 16, QLatin1Char('0'));
        }
        if (!result.isEmpty())
            result.chop(1); // 移除最后的空格
        return uppercase ? result.toUpper() : result;
    }
}

MessageBuilderUtils::MessageBuilderUtils(const QString &role, const QString &host, quint16 port)
    : m_role(role)
    , m_host(host)
    , m_port(port)
{
    //获取构造时间
    m_buildTime = QDateTime::currentDateTime().toString("hh:mm:ss");
}


QString MessageBuilderUtils::toHTMLText(const QString &msg, const QString &color)
{
    //先构造内部的字符串 前缀+时间+地址
    auto innerText = QString("<i>&lt;%1&gt;</i>[%2]<b>(%3:%4):</b>")
                         .arg(m_role, m_buildTime, m_host, QString::number(m_port));
    //再返回一个边框文本行
    return QString("<p>%2<span style='color:%1;font-weight:bold;'>%3</span></p>")
        .arg(color, innerText, msg.toHtmlEscaped());
}
