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

QString MessageBuilderUtils::toPlainText(const QString &msg)
{
    return QString("[%1](%2,%3:%4):%5").arg(m_buildTime).arg(m_role).arg(m_host).arg(m_port).arg(msg);
}

QString MessageBuilderUtils::toHTMLText(const QString &msg, const QString &color)
{
    return QString("<p>[%1](%2,%3:%4):<span style='color:%5'>%6</span></p>")
        .arg(m_buildTime)
        .arg(m_role)
        .arg(m_host)
        .arg(m_port)
        .arg(color)
        .arg(msg.toHtmlEscaped());
}
