#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>

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
    void onStartClicked();
    void onSendClicked();
    void onNewConnection();
    void onDisconnectClicked();


private:
    Ui::myWidget *ui;
    QTcpServer *tcpServer;
    QList<QTcpSocket*> clients;

    void appendLog(const QString &msg);
    void updateIPList();
    void updateClientList();

};
#endif // MYWIDGET_H







