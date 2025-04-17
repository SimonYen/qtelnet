#include "mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QHash>
#include <QNetworkProxyFactory>
#include <QRegularExpressionValidator>
#include "./ui_mainwindow.h"
#include "net/networkfactory.h"
#include "net/tcpserverhandler.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_mode(NetworkHandler::Mode::TCP_CLIENT)
    , m_handler(nullptr)
    , m_backspace_counter(0)
{
    ui->setupUi(this);
    //设置输入限制
    settingInputValidation();
    //禁止使用系统代理
    QNetworkProxyFactory::setUseSystemConfiguration(false);
    //安装事件过滤器
    qApp->installEventFilter(this);
    qInfo() << "Main Window constructed.";
}

MainWindow::~MainWindow()
{
    qInfo() << "Main Window deconstructed.";
    if (m_mode == NetworkHandler::Mode::TCP_SERVER) {
        auto handler = static_cast<TCPServerHandler *>(m_handler);
        handler->shutdown();
    }
    if (m_handler) {
        m_handler->close();
        delete m_handler;
    }
    delete ui;
}

//捕获键盘按下退格按钮
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->clientMessageTextEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Backspace) {
            //同时还是HEX模式
            if (ui->clientHexCheckBox->checkState() == Qt::Checked) {
                m_backspace_counter++;
            }
        }
    }
    return QObject::eventFilter(watched, event);
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
        break;
    case NetworkHandler::Mode::TCP_CLIENT: {
        //连接到客户端的槽
        connect(m_handler, &NetworkHandler::dataReceived, this, &MainWindow::onClientDataReceived);
        break;
    }
    case NetworkHandler::Mode::TCP_SERVER: {
        //基类指针转化
        auto server_handler = static_cast<TCPServerHandler *>(m_handler);
        //客户端到来
        connect(server_handler,
                &TCPServerHandler::clientComing,
                this,
                &MainWindow::onClientComboxAdded);
        //客户端离线
        connect(server_handler,
                &TCPServerHandler::clientLeaving,
                this,
                &MainWindow::onClientComboxLeft);
        //消息到来
        connect(server_handler,
                &TCPServerHandler::clientMessageSended,
                this,
                &MainWindow::onClientMessageReceived);
        break;
    }
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
    if (ui->clientDisplayButton->text() == "暂停显示") {
        MessageBuilderUtils mb("远程", m_handler->peerAddress(), m_handler->peerPort());
        ui->clientASCIITextBrowser->append(mb.toHTMLText(ASCIIString, "green"));
        ui->clientHEXTextBrowser->append(mb.toHTMLText(HEXString, "green"));
        ui->clientUTFTextBrowser->append(mb.toHTMLText(UTFString, "green"));
    }
}

void MainWindow::onClientMessageReceived(QPair<QHostAddress, quint16> address,
                                         const QByteArray &data)
{
    //数据转换
    auto UTFString = ByteArrayUtils::toUtf8String(data);
    auto ASCIIString = ByteArrayUtils::toAsciiString(data);
    auto HEXString = ByteArrayUtils::toHexString(data, true, true);

    //显示出来
    if (ui->serverDisplayButton->text() == "暂停显示") {
        MessageBuilderUtils mb("客户端", address.first.toString(), address.second);
        ui->serverASCIITextBrowser->append(mb.toHTMLText(ASCIIString, "green"));
        ui->serverHEXTextBrowser->append(mb.toHTMLText(HEXString, "green"));
        ui->serverUTFTextBrowser->append(mb.toHTMLText(UTFString, "green"));
    }
    ui->statusbar->showMessage(
        QString("收到客户端%1:%2发来的消息：%3")
            .arg(address.first.toString(), QString::number(address.second), UTFString));
    qInfo() << "Client" << address << " says: " << UTFString;

    auto handler = static_cast<TCPServerHandler *>(m_handler);
    //如果开启了自动回复
    if (ui->AutoCheckBox->isChecked()) {
        //获取发送框的消息
        auto text = ui->serverMessageTextEdit->toPlainText();
        //发送
        handler->sendToClientByAddress(address, text.toUtf8());
        //显示出来
        if (ui->serverDisplayButton->text() == "暂停显示") {
            MessageBuilderUtils mb("自动回复", handler->localAddress(), handler->localPort());
            ui->serverASCIITextBrowser->append(
                mb.toHTMLText(ByteArrayUtils::toAsciiString(text.toUtf8()), "pink"));
            ui->serverHEXTextBrowser->append(
                mb.toHTMLText(ByteArrayUtils::toHexString(text.toUtf8()), "pink"));
            ui->serverUTFTextBrowser->append(mb.toHTMLText(text, "pink"));
        }
    }
    //如果开启了Echo
    if (ui->EchoCheckBox->isChecked()) {
        //获取客户端发送的消息
        auto text = UTFString;
        //发送
        handler->sendToClientByAddress(address, text.toUtf8());
        //显示出来
        if (ui->serverDisplayButton->text() == "暂停显示") {
            MessageBuilderUtils mb("Echo", handler->localAddress(), handler->localPort());
            ui->serverASCIITextBrowser->append(
                mb.toHTMLText(ByteArrayUtils::toAsciiString(text.toUtf8()), "pink"));
            ui->serverHEXTextBrowser->append(
                mb.toHTMLText(ByteArrayUtils::toHexString(text.toUtf8()), "pink"));
            ui->serverUTFTextBrowser->append(mb.toHTMLText(text, "pink"));
        }
    }
    //如果开启了NTP
    if (ui->NTPCheckBox->isChecked()) {
        //获取当前时间
        auto text = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        //发送
        handler->sendToClientByAddress(address, text.toUtf8());
        //显示出来
        if (ui->serverDisplayButton->text() == "暂停显示") {
            MessageBuilderUtils mb("NTP", handler->localAddress(), handler->localPort());
            ui->serverASCIITextBrowser->append(
                mb.toHTMLText(ByteArrayUtils::toAsciiString(text.toUtf8()), "pink"));
            ui->serverHEXTextBrowser->append(
                mb.toHTMLText(ByteArrayUtils::toHexString(text.toUtf8()), "pink"));
            ui->serverUTFTextBrowser->append(mb.toHTMLText(text, "pink"));
        }
    }
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
                             .arg(NetworkHandler::mode2String(static_cast<int>(m_mode)),
                                  m_handler->localAddress(),
                                  QString::number(m_handler->localPort())));
}
//关闭连接
void MainWindow::on_closeConnectButton_clicked()
{
    if (m_mode == NetworkHandler::Mode::TCP_SERVER) {
        auto handler = static_cast<TCPServerHandler *>(m_handler);
        handler->shutdown();
    }
    //关闭对象
    m_handler->close();
    delete m_handler;
    m_handler = nullptr;
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
//TCP客户端点击发送消息按钮
void MainWindow::on_clientSendMessageButton_clicked()
{
    //获取需要发送的数据
    auto text = ui->clientMessageTextEdit->toPlainText();
    auto data = text.toUtf8();
    //如果是HEX模式
    if (ui->clientHexCheckBox->checkState() == Qt::Checked) {
        //先去掉空格
        text.remove(' ');
        //然后直接转成字节
        data = QByteArray::fromHex(text.toUtf8());
    }
    //一次性全部发送
    if (m_handler->writeAll(data)) {
        ui->statusbar->showMessage("发送成功");
        if (ui->clientDisplayButton->text() == "暂停显示") {
            //记录，首先转化
            auto UTFString = ByteArrayUtils::toUtf8String(data);
            auto ASCIIString = ByteArrayUtils::toAsciiString(data);
            auto HEXString = ByteArrayUtils::toHexString(data, true, true);
            MessageBuilderUtils mb("本地", m_handler->localAddress(), m_handler->localPort());
            //写入
            ui->clientUTFTextBrowser->append(mb.toHTMLText(UTFString, "blue"));
            ui->clientASCIITextBrowser->append(mb.toHTMLText(ASCIIString, "blue"));
            ui->clientHEXTextBrowser->append(mb.toHTMLText(HEXString, "blue"));
            qInfo() << "Sending message to " << m_handler->peerAddress() << ":"
                    << m_handler->peerPort() << ", message: " << text;
        }
    } else {
        ui->statusbar->showMessage("发送失败！");
    }
}

void MainWindow::on_clientDisplayButton_clicked()
{
    if (ui->clientDisplayButton->text() == "暂停显示") {
        ui->clientDisplayButton->setText("恢复显示");
        ui->statusbar->showMessage("已暂停显示消息");
    } else {
        ui->clientDisplayButton->setText("暂停显示");
        ui->statusbar->showMessage("已恢复显示消息");
    }
}
//清空消息
void MainWindow::on_clientClearMessageButton_clicked()
{
    ui->clientASCIITextBrowser->clear();
    ui->clientHEXTextBrowser->clear();
    ui->clientUTFTextBrowser->clear();
    ui->statusbar->showMessage("已清空消息");
    qInfo() << "Clear Messages";
}
//保存消息
void MainWindow::on_clientSaveMessageButton_clicked()
{
    // 弹出文件夹选择对话框
    QString folderPath = QFileDialog::getExistingDirectory(this, "想将消息保存在何处？");
    if (folderPath.isEmpty()) {
        ui->statusbar->showMessage("未选择保存目录");
        return;
    }
    //获取当前Tab
    int current_index = ui->clientTabWidget->currentIndex();
    QTextBrowser *current_browser = nullptr;
    //说实话，这里写法很粗糙
    QHash<int, QTextBrowser *> index2browser = {
        {0, ui->clientUTFTextBrowser},
        {1, ui->clientASCIITextBrowser},
        {2, ui->clientHEXTextBrowser},
    };
    current_browser = index2browser[current_index];
    if (current_browser == nullptr) {
        qWarning() << "The pointer of browser is NULL!!! " << "index: " << current_index;
        return;
    }
    //构建生成文件名
    QDir dir(folderPath);
    auto file_name = dir.absoluteFilePath(ui->clientTabWidget->tabText(current_index) + ".txt");
    QFile file(file_name);
    //尝试打开文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "Creating file failed, filename: " << file_name;
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << current_browser->toPlainText();
    qInfo() << "Messsages saved. File location: " << file_name;
    file.close();
    ui->statusbar->showMessage("消息记录保存成功，保存在：" + file_name);
}

//客户端HEX状态改变
void MainWindow::on_clientHexCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    //如果处于HEX编辑模式
    if (arg1 == Qt::Checked) {
        //原有UTF8字符串转化为HEX进制字符串
        auto UTF8Text = ui->clientMessageTextEdit->toPlainText();
        //覆盖
        ui->clientMessageTextEdit->setPlainText(
            ByteArrayUtils::toHexString(UTF8Text.toUtf8(), true, true));
    } else {
        //HEX转为UTF8
        auto HexText = ui->clientMessageTextEdit->toPlainText();
        QByteArray hex = QByteArray::fromHex(HexText.remove(' ').toUtf8()); // 去掉空格再转
        //覆盖
        ui->clientMessageTextEdit->setPlainText(QString::fromUtf8(hex));
    }
    // 光标移到文本末尾
    QTextCursor cursor = ui->clientMessageTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->clientMessageTextEdit->setTextCursor(cursor);
}

void MainWindow::on_clientMessageTextEdit_textChanged()
{
    // 避免递归触发
    static bool inProcess = false;
    if (inProcess)
        return;

    //如果不是HEX模式，不管
    if (ui->clientHexCheckBox->checkState() != Qt::Checked) {
        return;
    }

    inProcess = true;

    //取出原始字符
    auto rawText = ui->clientMessageTextEdit->toPlainText();
    //如果是按下退格键
    if (m_backspace_counter) {
        rawText.chop(m_backspace_counter);
        m_backspace_counter = 0;
        ui->clientHEXTextBrowser->setPlainText(rawText);
        // 重置状态
        inProcess = false;
        // 光标移到文本末尾
        QTextCursor cursor = ui->clientMessageTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->clientMessageTextEdit->setTextCursor(cursor);
        return;
    }
    // 只保留十六进制字符
    rawText.remove(QRegularExpression("[^0-9A-Fa-f]"));
    //最终结果
    QString text = "";
    //每两位插一个空格
    int counter = 0;
    for (const auto ch : rawText) {
        text.append(ch);
        counter++;
        if (counter == 2) {
            //插入空格并重新置零
            text.append(' ');
            counter = 0;
        }
    }
    //直接自动转大写
    ui->clientMessageTextEdit->setPlainText(text.toUpper());

    // 重置状态
    inProcess = false;

    // 光标移到文本末尾
    QTextCursor cursor = ui->clientMessageTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->clientMessageTextEdit->setTextCursor(cursor);
}

void MainWindow::onClientComboxAdded(QPair<QHostAddress, quint16> address)
{
    QString host = QString("%1:%2").arg(address.first.toString()).arg(address.second);
    ui->clientComboBox->addItem(host);
    ui->statusbar->showMessage(QString("新客户端%1已到来").arg(host));
}

void MainWindow::onClientComboxLeft(QPair<QHostAddress, quint16> address)
{
    //从多选框里找到这一项
    QString host = QString("%1:%2").arg(address.first.toString()).arg(address.second);
    int index = ui->clientComboBox->findText(host);
    if (index != -1) {
        ui->clientComboBox->removeItem(index);
        ui->statusbar->showMessage(QString("客户端%1已离线").arg(host));
    }
}
//TCP服务器点击回复消息按钮
void MainWindow::on_serverReplyButton_clicked()
{
    auto handler = static_cast<TCPServerHandler *>(m_handler);
    //获取需要回复的内容
    auto message = ui->serverMessageTextEdit->toPlainText();
    auto data = message.toUtf8();
    //记录，首先转化
    auto UTFString = ByteArrayUtils::toUtf8String(data);
    auto ASCIIString = ByteArrayUtils::toAsciiString(data);
    auto HEXString = ByteArrayUtils::toHexString(data, true, true);
    MessageBuilderUtils *mb = nullptr;
    //判断是否广播
    if (ui->clientComboBox->currentIndex() == 0) {
        //广播，群发
        handler->sendToAllClient(message.toUtf8());
        ui->statusbar->showMessage("已将消息发送给所有客户端");
        //构造消息记录
        mb = new MessageBuilderUtils("服务器=>全体",
                                     m_handler->localAddress(),
                                     m_handler->localPort());
    } else {
        //单发的话，获取地址
        auto host = ui->clientComboBox->currentText();
        //构造
        QPair<QHostAddress, quint16> addr = {QHostAddress(host.split(":")[0]),
                                             host.split(":")[1].toInt()};
        handler->sendToClientByAddress(addr, message.toUtf8());
        ui->statusbar->showMessage("单独发送消息给" + host);
        //构造消息记录
        mb = new MessageBuilderUtils("服务器=>" + host,
                                     m_handler->localAddress(),
                                     m_handler->localPort());
    }

    //写入
    if (ui->serverDisplayButton->text() == "暂停显示") {
        ui->serverUTFTextBrowser->append(mb->toHTMLText(UTFString, "blue"));
        ui->serverASCIITextBrowser->append(mb->toHTMLText(ASCIIString, "blue"));
        ui->serverHEXTextBrowser->append(mb->toHTMLText(HEXString, "blue"));
    }
    qInfo() << "Replying message: " << message;
    delete mb;
}

void MainWindow::on_serverDisplayButton_clicked()
{
    if (ui->serverDisplayButton->text() == "暂停显示") {
        ui->serverDisplayButton->setText("继续显示");
        ui->statusbar->showMessage("停止显示消息记录");
    } else {
        ui->serverDisplayButton->setText("暂停显示");
        ui->statusbar->showMessage("恢复消息记录显示");
    }
}

void MainWindow::on_serverMessageSaveButton_clicked()
{
    // 弹出文件夹选择对话框
    QString folderPath = QFileDialog::getExistingDirectory(this, "想将消息保存在何处？");
    if (folderPath.isEmpty()) {
        ui->statusbar->showMessage("未选择保存目录");
        return;
    }
    //获取当前Tab
    int current_index = ui->serverTabWidget->currentIndex();
    QTextBrowser *current_browser = nullptr;
    //说实话，这里写法很粗糙
    QHash<int, QTextBrowser *> index2browser = {
        {0, ui->serverUTFTextBrowser},
        {1, ui->serverASCIITextBrowser},
        {2, ui->serverHEXTextBrowser},
    };
    current_browser = index2browser[current_index];
    if (current_browser == nullptr) {
        qWarning() << "The pointer of browser is NULL!!! " << "index: " << current_index;
        return;
    }
    //构建生成文件名
    QDir dir(folderPath);
    auto file_name = dir.absoluteFilePath(ui->serverTabWidget->tabText(current_index) + ".txt");
    QFile file(file_name);
    //尝试打开文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "Creating file failed, filename: " << file_name;
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << current_browser->toPlainText();
    qInfo() << "Messsages saved. File location: " << file_name;
    file.close();
    ui->statusbar->showMessage("消息记录保存成功，保存在：" + file_name);
}

void MainWindow::on_serverMessageClearButton_clicked()
{
    ui->serverASCIITextBrowser->clear();
    ui->serverHEXTextBrowser->clear();
    ui->serverUTFTextBrowser->clear();
    ui->statusbar->showMessage("消息清空成功");
}
