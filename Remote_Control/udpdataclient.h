#ifndef UDPDATACLIENT_H
#define UDPDATACLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class UdpDataClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit UdpDataClient(QObject *parent = nullptr);

    Q_INVOKABLE void connectToHost(const QString &ip, quint16 port);
    Q_INVOKABLE void disconnectFromHost();
    Q_INVOKABLE void sendMessage(const QString &data);

    bool isConnected() const;

signals:
    void dataReceived(const QByteArray &data);
    void connectedChanged();

private slots:
    void onReadyRead();

private:
    QUdpSocket *socket;
    QHostAddress targetIp;
    quint16 targetPort;
    bool activeState; // 维护应用层的虚拟连接状态
};

#endif // UDPDATACLIENT_H
