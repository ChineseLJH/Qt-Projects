#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVBoxLayout>
#include <QGridLayout>

QT_BEGIN_NAMESPACE
namespace Ui {
class myWidget;
}
QT_END_NAMESPACE

class myWidget : public QWidget
{
    Q_OBJECT

public:
    myWidget(QWidget *parent = nullptr);
    ~myWidget();

private slots:
    void updateSerialPortList();  // 刷新串口列表
    void on_CboxSerialPort_1_currentTextChanged(const QString &arg1);  // 选择串口
    void openSerialPort();  // 打开或关闭串口
    void sendData();  // 发送数据
    void appendMessage(const QString &message);
    void readSerialData();
    void clearReceivedData();
    void on_checkBox_1_toggled(bool checked);
    void on_checkBox_2_toggled(bool checked);
    void on_pushButton_5_clicked();


private:
    Ui::myWidget *ui;
    QSerialPort mSerialPort;
    bool displayHex_1 = false;
    bool displayHex_2 = false;
};

#endif // MYWIDGET_H
