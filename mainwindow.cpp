#include "mainwindow.h"
#include <QDebug>
#include <QNetworkProxyFactory>
#include <QRegularExpressionValidator>
#include "./ui_mainwindow.h"
#include "net/networkfactory.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_mode(NetworkHandler::Mode::TCP_CLIENT)
    , m_handler(nullptr)
{
    ui->setupUi(this);
    //设置输入限制
    settingInputValidation();
    //禁止使用系统代理
    QNetworkProxyFactory::setUseSystemConfiguration(false);
    qInfo() << "Main Window constructed.";
}

MainWindow::~MainWindow()
{
    qInfo() << "Main Window deconstructed.";
    if (m_handler)
        delete m_handler;
    delete ui;
}
//当套接字类型选择框有所改变的时候
void MainWindow::on_socketTypeCombox_currentIndexChanged(int index)
{
    //切换成员变量
    m_mode = static_cast<NetworkHandler::Mode>(index);
    //输出出来
    ui->statusbar->showMessage("工具切换到：" + NetworkHandler::mode2String(index));
    qInfo() << "Switching to " << NetworkHandler::mode2String(index);
}

void MainWindow::settingInputValidation()
{
    // 定义正则表达式：允许0或非零开头的1-65535
    QRegularExpression portRegExp(
        "^(0|[1-9]\\d{0,3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])$");
    // 创建端口验证器
    QRegularExpressionValidator *portValidator = new QRegularExpressionValidator(portRegExp, this);
    ui->portLineEdit->setValidator(portValidator);
}
//开始连接
void MainWindow::on_startConnectButton_clicked()
{
    qInfo() << "Prepare to connect...";
    //获取地址和端口
    auto host = ui->hostLineEdit->text();
    auto port = ui->portLineEdit->text().toInt();
    //如果地址为空，那就采用本地loop地址
    if (host.isEmpty())
        host = "127.0.0.1";
    ui->statusbar->showMessage(QString("开始连接，host: %1，port: %2").arg(host).arg(port));
    m_address = {QHostAddress(host), port};

    if (m_handler)
        delete m_handler;
    //初始化对象
    m_handler = NetworkFactory::createHandler(m_mode, this);
    //检测连接状态
    if (!(m_handler->init(m_address))) {
        ui->statusbar->showMessage("连接失败！！！");
        m_handler->close();
        //立即返回
        return;
    }
    //连接网络信号和槽

    //更改按钮属性
    ui->startConnectButton->setText("连接中");
    ui->startConnectButton->setEnabled(false);
    ui->closeConnectButton->setEnabled(true);
}
//关闭连接
void MainWindow::on_closeConnectButton_clicked()
{
    //关闭对象
    m_handler->close();
    ui->statusbar->showMessage("连接已关闭");
    m_address = {QHostAddress(), 0};
    //还原按钮属性
    ui->startConnectButton->setText("开始连接");
    ui->startConnectButton->setEnabled(true);
    ui->closeConnectButton->setEnabled(false);
    qInfo() << "Connection closed.";
}
