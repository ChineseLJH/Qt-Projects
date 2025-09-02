#ifndef MYUDP_H
#define MYUDP_H

#include <QObject>
#include <QUdpSocket>

class MyUdp : public QObject
{
    Q_OBJECT
public:
    explicit MyUdp(QObject *parent = nullptr);
    Q_INVOKABLE void startListening(quint16 port = 12345);  // 可从 QML 调用

signals:
    void ipReceived(QString ip);

private slots:
    void onReadyRead();

private:
    QUdpSocket *udpSocket;
};

#endif // MYUDP_H
