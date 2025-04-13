#include "mainwindow.h"
#include <QDebug>
#include <QNetworkProxyFactory>
#include <QRegularExpressionValidator>
#include "./ui_mainwindow.h"
#include "net/networkfactory.h"
#include "utils.h"

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

void MainWindow::connectingNetworkSignalsAndSlots()
{
    //判断当前模式
    switch (m_mode) {
    case NetworkHandler::Mode::UDP:
    case NetworkHandler::Mode::TCP_CLIENT: {
        //连接到客户端的槽
        connect(m_handler, &NetworkHandler::dataReceived, this, &MainWindow::onClientDataReceived);
        break;
    }
    //服务器模式日后实现
    case NetworkHandler::Mode::TCP_SERVER:
        break;
    default:
        break;
    }
    //三个模式错误处理都是一样的
    connect(m_handler, &NetworkHandler::errorOccurred, this, &MainWindow::onSocketErrorOccurred);
}

void MainWindow::onClientDataReceived(const QByteArray &data)
{
    //数据转换
    auto UTFString = ByteArrayUtils::toUtf8String(data);
    auto ASCIIString = ByteArrayUtils::toAsciiString(data);
    auto HEXString = ByteArrayUtils::toHexString(data, true, true);

    //显示出来
    MessageBuilderUtils mb("远程", m_handler->peerAddress(), m_handler->peerPort());
    ui->clientASCIITextBrowser->append(mb.toHTMLText(ASCIIString, "green"));
    ui->clientHEXTextBrowser->append(mb.toHTMLText(HEXString, "green"));
    ui->clientUTFTextBrowser->append(mb.toHTMLText(UTFString, "green"));

    //数据包统计
}

void MainWindow::onSocketErrorOccurred(const QString &error)
{
    //直接状态栏显示
    ui->statusbar->showMessage("错误发生：" + error);
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
    connectingNetworkSignalsAndSlots();

    //更改按钮属性
    ui->startConnectButton->setText("连接中");
    ui->startConnectButton->setEnabled(false);
    ui->closeConnectButton->setEnabled(true);
    ui->clientSendMessageButton->setEnabled(true);

    //更改标题
    this->setWindowTitle(QString("Qtelnet | %1 | 本机[%2:%3]")
                             .arg(NetworkHandler::mode2String(static_cast<int>(m_mode)))
                             .arg(m_handler->localAddress())
                             .arg(m_handler->localPort()));
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
    ui->clientSendMessageButton->setEnabled(false);
    qInfo() << "Connection closed.";
    //恢复标题
    this->setWindowTitle("Qtelnet");
}
//TCP客户端/UDP 模式点击发送消息按钮
void MainWindow::on_clientSendMessageButton_clicked()
{
    //获取需要发送的数据
    auto text = ui->clientMessageTextEdit->toPlainText();
    auto data = text.toUtf8();
    //一次性全部发送
    if (m_handler->writeAll(data)) {
        ui->statusbar->showMessage("发送成功");
        //记录，首先转化两次
        auto ASCIIString = ByteArrayUtils::toAsciiString(data);
        auto HEXString = ByteArrayUtils::toHexString(data, true, true);
        MessageBuilderUtils mb("本地", m_handler->localAddress(), m_handler->localPort());
        //写入
        ui->clientUTFTextBrowser->append(mb.toHTMLText(text, "blue"));
        ui->clientASCIITextBrowser->append(mb.toHTMLText(ASCIIString, "blue"));
        ui->clientHEXTextBrowser->append(mb.toHTMLText(HEXString, "blue"));
    } else {
        ui->statusbar->showMessage("发送失败！");
    }
}
