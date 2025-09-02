#include "BluetoothManager.h"
#include <QDebug>
#include <QBluetoothLocalDevice>
#include <QtBluetooth/QtBluetooth>
// #include <QDBusReply>
#include <QtCore/QLoggingCategory>

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent),
    m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this)),
    m_socket(new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this))
{

    if (QBluetoothLocalDevice::allDevices().isEmpty()) {
        qDebug() << "无可用蓝牙设备";
    } else {
        qDebug() << "找到蓝牙设备:" << QBluetoothLocalDevice::allDevices().first().name();
    }

    qDebug() << "Supported Bluetooth adapters:";
    const QList<QBluetoothHostInfo> devices = QBluetoothLocalDevice::allDevices();
    for (const QBluetoothHostInfo &info : devices) {
        qDebug() << info.name() << info.address().toString();
    }

    // 扫描到设备
    connect(m_discoveryAgent,
            &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this,
            &BluetoothManager::onDeviceDiscovered);

    // 连接状态变化
    connect(m_socket,
            &QBluetoothSocket::connected,
            this,
            &BluetoothManager::onSocketConnected);
    connect(m_socket,
            &QBluetoothSocket::disconnected,
            this,
            &BluetoothManager::onSocketDisconnected);

    // 收到数据
    connect(m_socket,
            &QBluetoothSocket::readyRead,
            this,
            &BluetoothManager::onSocketReadyRead);

    // 错误信号（Qt6 已改名为 errorOccurred）
    connect(m_socket,
            &QBluetoothSocket::errorOccurred,
            this,
            &BluetoothManager::onSocketError);
}

BluetoothManager::~BluetoothManager()
{
    delete m_localDevice;
    if (m_socket->isOpen())
        m_socket->close();
}


void BluetoothManager::startScan()
{
    m_discoveryAgent->start();
}

void BluetoothManager::onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    QString address = info.address().toString();
    emit deviceDiscovered(info.name(), address);

    if (address == targetAddress()) {
        qDebug() << "发现目标设备:" << info.name() << address;
        setTargetAddress(address);
        registerPairingAgent(); // 移除参数
    }
}

void BluetoothManager::connectToDevice(const QString &address, const QString &uuidStr)
{
    if (m_socket->isOpen())
        m_socket->close();

    QBluetoothAddress addr(address);
    QBluetoothUuid uuid(uuidStr);
    m_socket->connectToService(addr, uuid);
}


void BluetoothManager::disconnect()
{
    m_socket->disconnectFromService();
}

void BluetoothManager::sendMessage(const QString &msg) {
    if (m_socket->state() == QBluetoothSocket::SocketState::ConnectedState) {
        m_socket->write(msg.toUtf8()); // 正确转换为UTF-8字节流
    }
}


QString BluetoothManager::targetAddress() const {
    return m_targetAddress;
}

void BluetoothManager::setTargetAddress(const QString &address) {
    if (m_targetAddress != address) {
        m_targetAddress = address;
        emit targetAddressChanged();
    }
}



void BluetoothManager::onSocketConnected()
{
    emit connected();
}

void BluetoothManager::onSocketDisconnected()
{
    emit disconnected();
}

// void BluetoothManager::onSocketReadyRead()
// {
//     QByteArray data = m_socket->readAll();
//     emit dataReceived(data);
// }

// 修改BluetoothManager.cpp中的onSocketReadyRead函数
void BluetoothManager::onSocketReadyRead() {
    QByteArray data = m_socket->readAll();
    emit dataReceived(data); // ✅ 直接发送原始字节
}

void BluetoothManager::onSocketError(const QBluetoothSocket::SocketError &err)
{
    Q_UNUSED(err)
    emit errorOccurred(m_socket->errorString());
}


// 更新配对结果处理
void BluetoothManager::registerPairingAgent()
{
    if (!m_localDevice) {
        m_localDevice = new QBluetoothLocalDevice(this);
    }

    const QBluetoothAddress targetAddr(targetAddress());
    if (targetAddr.isNull()) {
        emit errorOccurred("无效设备地址");
        return;
    }

    static QMetaObject::Connection pairingConn;
    if (pairingConn) QObject::disconnect(pairingConn);
    pairingConn = connect(m_localDevice, &QBluetoothLocalDevice::pairingFinished,
                          [this](const QBluetoothAddress &addr, QBluetoothLocalDevice::Pairing status) {
                              Q_UNUSED(addr) // 消除警告
                              if (status == QBluetoothLocalDevice::Paired) {
                                  emit paired();
                              } else {
                                  // 使用通用错误提示
                                  emit errorOccurred(tr("配对失败，请确认设备处于可配对状态"));
                              }
                          });

    m_localDevice->requestPairing(targetAddr, QBluetoothLocalDevice::Paired);
}
