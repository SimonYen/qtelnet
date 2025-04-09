#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

private slots:
    void on_socketTypeCombox_currentIndexChanged(int index);

    void on_startConnectButton_clicked();

    void on_closeConnectButton_clicked();

private:
    Ui::MainWindow *ui;
    //当前所处的模式（工具所选择的套接字类型）
    NetworkHandler::Mode m_mode;
    //IP地址
    QPair<QString, int> m_address;

private:
    //设置输入验证
    void settingInputValidation();
};
#endif // MAINWINDOW_H
