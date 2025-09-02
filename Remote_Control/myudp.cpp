#include "myudp.h"
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QDebug>

MyUdp::MyUdp(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
    connect(udpSocket, &QUdpSocket::readyRead, this, &MyUdp::onReadyRead);
}

void MyUdp::startListening(quint16 port)
{
    if (!udpSocket->bind(QHostAddress::Any, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qWarning() << "Failed to bind UDP port";
    } else {
        qDebug() << "UDP listening on port" << port;
    }
}

void MyUdp::onReadyRead()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QString senderIp = datagram.senderAddress().toString();
        qDebug() << "UDP received from:" << senderIp;
        emit ipReceived(senderIp);  // 发信号给 QML
    }
}
