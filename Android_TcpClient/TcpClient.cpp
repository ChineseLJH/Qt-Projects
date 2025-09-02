#include "TcpClient.h"
#include <QHostAddress>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent),
    m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);

    // scan 超时处理
    m_scanTimer.setSingleShot(true);
    m_scanTimer.setInterval(2000); // 2 秒超时
    connect(&m_scanTimer, &QTimer::timeout, this, &TcpClient::onScanTimeout);
}

void TcpClient::scan()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState)
        m_socket->disconnectFromHost();

    m_available = false;
    emit deviceAvailableChanged();

    m_scanTimer.start();
    m_socket->connectToHost(QHostAddress(m_host), m_port);
}

void TcpClient::onScanTimeout()
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->abort();
        m_available = false;
        emit deviceAvailableChanged();
    }
}

void TcpClient::onConnected()
{
    if (m_scanTimer.isActive()) {
        // 扫描模式下发现设备
        m_scanTimer.stop();
        m_available = true;
        emit deviceAvailableChanged();
        m_socket->disconnectFromHost(); // 关闭临时连接
    } else {
        // 正式连接成功
        emit connected();
    }
}

void TcpClient::connectToDevice()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState)
        return;
    m_socket->connectToHost(m_host, m_port);
}

void TcpClient::onDisconnected()
{
    emit disconnected();
}

void TcpClient::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    emit messageReceived(QString::fromUtf8(data));
}

void TcpClient::sendMessage(const QString &msg)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(msg.toUtf8());
    }
}
