// mywidget.h
#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QNetworkInterface>

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
    void on_pushButton_1_clicked();
    void on_pushButton_2_clicked();
    void onReadyRead();

private:
    Ui::myWidget *ui;
    QUdpSocket *udpSocket;
    bool isConnected = false;
    quint16 currentPort = 0;

    void appendLog(const QString &message);
    void updateIPList();
};

#endif // MYWIDGET_H
