#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>

class TcpClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)  // 注意属性名

public:
    explicit TcpClient(QObject *parent = nullptr);

    // 修改为 Q_INVOKABLE 使它们可以被 QML 调用
    Q_INVOKABLE void connectToHost(const QString &ip, quint16 port);
    Q_INVOKABLE void disconnectFromHost();
    Q_INVOKABLE void sendMessage(const QString &data);


    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void connectedChanged();  // 用于 Q_PROPERTY

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *socket;
};

#endif // TCPCLIENT_H
