#ifndef TCPCLIENT_H
#define TCPCLIENT_H



#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class TcpClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool deviceAvailable READ deviceAvailable NOTIFY deviceAvailableChanged)
public:
    explicit TcpClient(QObject *parent = nullptr);

    Q_INVOKABLE void scan();            // 尝试探测 ESP32
    Q_INVOKABLE void connectToDevice(); // 发起正式连接
    Q_INVOKABLE void sendMessage(const QString &msg);

    bool deviceAvailable() const { return m_available; }

signals:
    void deviceAvailableChanged();
    void connected();
    void disconnected();
    void messageReceived(const QString &msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onScanTimeout();

private:
    QTcpSocket *m_socket;
    QTimer m_scanTimer;
    bool m_available = false;

    const QString m_host = "192.168.4.1";
    const quint16 m_port = 12345;
};



#endif // TCPCLIENT_H


