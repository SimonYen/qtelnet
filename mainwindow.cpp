#include "mainwindow.h"
#include <QRegularExpressionValidator>
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_socketType(SocketType::TCP_CLIENT)
{
    ui->setupUi(this);
    //设置输入限制
    settingInputValidation();
}

MainWindow::~MainWindow()
{
    delete ui;
}
//当套接字类型选择框有所改变的时候
void MainWindow::on_socketTypeCombox_currentIndexChanged(int index)
{
    //切换成员变量
    m_socketType = static_cast<SocketType>(index);
    //输出出来
    ui->statusbar->showMessage("工具切换到：" + SocketTypeToString(index));
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
    //获取地址和端口
    auto host = ui->hostLineEdit->text();
    auto port = ui->portLineEdit->text().toInt();
    //如果地址为空，那就采用本地loop地址
    if (host.isEmpty())
        host = "127.0.0.1";
    ui->statusbar->showMessage(QString("开始连接，host: %1，port: %2").arg(host).arg(port));
    m_address = {host, port};
    //初始化对象

    //更改按钮属性
    ui->startConnectButton->setText("连接中");
    ui->startConnectButton->setEnabled(false);
    ui->closeConnectButton->setEnabled(true);
}
//关闭连接
void MainWindow::on_closeConnectButton_clicked()
{
    //关闭对象

    ui->statusbar->showMessage("连接已关闭");
    m_address = {"", 0};
    //还原按钮属性
    ui->startConnectButton->setText("开始连接");
    ui->startConnectButton->setEnabled(true);
    ui->closeConnectButton->setEnabled(false);
}
