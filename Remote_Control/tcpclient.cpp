#include "tcpclient.h"
#include <QDebug>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent),
    socket(new QTcpSocket(this))
{
    connect(socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &TcpClient::onErrorOccurred);
}

void TcpClient::connectToHost(const QString &ip, quint16 port)
{
    if (socket->state() == QAbstractSocket::ConnectedState)
        socket->disconnectFromHost();

    socket->connectToHost(ip, port);
}

void TcpClient::disconnectFromHost()
{
    if (socket->state() == QAbstractSocket::ConnectedState)
        socket->disconnectFromHost();
}

void TcpClient::sendMessage(const QString &data)
{
    if (socket->state() == QAbstractSocket::ConnectedState)
        socket->write(data.toUtf8());
}


bool TcpClient::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

void TcpClient::onConnected()
{
    emit connected();
    emit connectedChanged();
    qDebug() << "TCP connected.";
}

void TcpClient::onDisconnected()
{
    emit disconnected();
    emit connectedChanged();
    qDebug() << "TCP disconnected.";
}

void TcpClient::onReadyRead()
{
    QByteArray data = socket->readAll();
    emit dataReceived(data);
    qDebug() << "TCP received:" << data;
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit errorOccurred(socket->errorString());
    qDebug() << "TCP error:" << socket->errorString();
}

