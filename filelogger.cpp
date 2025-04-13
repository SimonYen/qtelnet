#include "filelogger.h"
#include <QDateTime>
#include <QDebug>

FileLogger::FileLogger()
{
    //附加模式打开文件
    m_logFile.setFileName("log.txt");
    if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        qFatal() << "Failed to open log file !";
    }
    m_stream.setDevice(&m_logFile);
}

FileLogger::~FileLogger()
{
    m_logFile.close();
}

void FileLogger::logMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    //获取当前时间
    auto time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    //构建日志格式
    auto log = QString("[%1][%2]:%3").arg(messageTypeToString(type)).arg(time).arg(msg);
    //以UTF-8格式保存
    m_stream << log.toUtf8() << Qt::endl;
}

QString FileLogger::messageTypeToString(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return "DEBU";
    case QtInfoMsg:
        return "INFO";
    case QtWarningMsg:
        return "WARN";
    case QtCriticalMsg:
        return "CRIT";
    case QtFatalMsg:
        return "FATA";
    }
    return "UNKNOWN";
}
