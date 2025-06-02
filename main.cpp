#include "filelogger.h"
#include "mainwindow.h"

#include <QApplication>
#ifdef Q_OS_WIN
#include <QtLogging>
#else
#include <qlogging.h>
#endif

//定义全局文件日志对象
FileLogger *logger = nullptr;

void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (logger)
        logger->logMessage(type, context, msg);
}

int main(int argc, char *argv[])
{
    //初始化日志对象
    logger = new FileLogger();
    //注册日志消息处理器
    qInstallMessageHandler(logToFile);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
