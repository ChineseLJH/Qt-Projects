#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QDateTime>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkInterface>

QT_BEGIN_NAMESPACE
namespace Ui { class myWidget; }
QT_END_NAMESPACE

class myWidget : public QWidget
{
    Q_OBJECT

public:
    myWidget(QWidget *parent = nullptr);
    ~myWidget();

private slots:
    void on_pushButton_TCP_Client_clicked();
    void on_pushButton_TCP_Server_clicked();
    void on_pushButton_UDP_clicked();

    void on_pushButton_1_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

private:
    Ui::myWidget *ui;

    int mode = 0; // 0: 未选择，1: TCP客户端，2: TCP服务端，3: UDP
    bool state=false;//false为没有连接，true为已有连接
    QTcpServer *tcpServer = nullptr;
    QList<QTcpSocket*> tcpClients;
    QTcpSocket *tcpSocket = nullptr;

    QUdpSocket *udpSocket = nullptr;

    void updateLocalIPList();
    void updateLocalIPList(QComboBox *comboBox);
    void updateClientList();
    void updateTcpClientList();
    void appendLog(const QString &message);
    void closeAllConnections();

    QTcpSocket* getSelectedTcpClient();
    QTcpSocket* findClientByAddress(const QString &address);
};

#endif // MYWIDGET_H
