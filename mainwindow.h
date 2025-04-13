#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHostAddress>
#include <QMainWindow>
#include <QPair>
#include "net/networkhandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_socketTypeCombox_currentIndexChanged(int index);

    void on_startConnectButton_clicked();

    void on_closeConnectButton_clicked();

    void on_clientSendMessageButton_clicked();

private:
    Ui::MainWindow *ui;
    //当前所处的模式（工具所选择的套接字类型）
    NetworkHandler::Mode m_mode;
    //IP地址
    QPair<QHostAddress, int> m_address;
    //网络句柄
    NetworkHandler *m_handler;
    //Hex编辑器模式下按下的退格数
    int m_backspace_counter;

private:
    //设置输入验证
    void settingInputValidation();
    //连接网络方面的信号和槽
    void connectingNetworkSignalsAndSlots();

    //自定义槽
private slots:
    //客户端消息收到时
    void onClientDataReceived(const QByteArray &data);
    //发生错误时
    void onSocketErrorOccurred(const QString &error);
    void on_clientDisplayButton_clicked();
    void on_clientClearMessageButton_clicked();
    void on_clientSaveMessageButton_clicked();
    void on_clientHexCheckBox_checkStateChanged(const Qt::CheckState &arg1);
    void on_clientMessageTextEdit_textChanged();
};
#endif // MAINWINDOW_H
