#include "udpdataclient.h"
#include <QNetworkDatagram>

UdpDataClient::UdpDataClient(QObject *parent)
    : QObject(parent), socket(new QUdpSocket(this)), activeState(false)
{
    // 绑定本地任意可用端口，以便操作系统内核能够将 ESP32 发回的 ACK 数据报路由至此套接字
    socket->bind(QHostAddress::Any, 0);
    connect(socket, &QUdpSocket::readyRead, this, &UdpDataClient::onReadyRead);
}

void UdpDataClient::connectToHost(const QString &ip, quint16 port)
{
    targetIp = QHostAddress(ip);
    targetPort = port;
    activeState = true;
    emit connectedChanged(); // 触发 QML 端状态更新，启动 50Hz 定时器
}

void UdpDataClient::disconnectFromHost()
{
    activeState = false;
    emit connectedChanged(); // 触发 QML 端状态更新，停止定时器
}

void UdpDataClient::sendMessage(const QString &data)
{
    if (activeState) {
        // 调用内核 API，将负载数据封装进 UDP 数据报并交由 IP 层路由
        socket->writeDatagram(data.toUtf8(), targetIp, targetPort);
    }
}

bool UdpDataClient::isConnected() const
{
    return activeState;
}

void UdpDataClient::onReadyRead()
{
    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        emit dataReceived(datagram.data()); // 将提取的 payload（丢包率分子 num）传递给 QML
    }
}
