#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QFile>
#include <QObject>
#include <QStringConverter>
#include <QTextStream>

class FileLogger : public QObject
{
private:
    QFile m_logFile;
    QTextStream m_stream;

public:
    FileLogger();
    ~FileLogger();
    //格式化输出日志
    void logMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    // 转换日志类型为字符串
    QString messageTypeToString(QtMsgType type);
};

#endif // FILELOGGER_H
