#include "bluetoothmanager.h"
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

    // 错误信号
    connect(m_socket,
            &QBluetoothSocket::errorOccurred,
            this,
            &BluetoothManager::onSocketError);
}

BluetoothManager::~BluetoothManager()
{
    // delete m_localDevice;
    if (m_socket->isOpen())
    {
        m_socket->abort();
        m_socket->close();
    }
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

void BluetoothManager::abortconnection()
{
    if(m_socket)
    {
        m_socket->abort();
        m_socket->close();
    }
}

void BluetoothManager::sendMessage(const QString &msg) {
    if(!m_socket)
    {
        return ;
    }

    if (m_socket->state() == QBluetoothSocket::SocketState::ConnectedState) {
        m_socket->write(msg.toUtf8()); // 正确转换为UTF-8字节流
    }
}

void BluetoothManager::sendControlData(double angle, double dist,
                                       bool btnA, bool btnB, bool btnC, bool btnD)
{
    // 1. 检查连接状态 (借用 sendMessage 里的检查逻辑，或者直接在这里判断)
    if (!m_socket || m_socket->state() != QBluetoothSocket::SocketState::ConnectedState) {
        return;
    }

    // 2. 格式化数据体
    // 对应 JS: `[${angle};${dist};${a};${b};${c};${d}]`
    // %.1f 保留1位小数, %.2f 保留2位小数, %d 对应整数(0/1)
    QString dataPart = QString::asprintf("[%.1f;%.2f;%d;%d;%d;%d]",
                                         angle, dist,
                                         btnA ? 1 : 0,
                                         btnB ? 1 : 0,
                                         btnC ? 1 : 0,
                                         btnD ? 1 : 0);

    // 3. 计算 CRC (将 QString 转为 QByteArray 进行计算)
    QString crcHex = calculateCRC16(dataPart.toUtf8());

    // 4. 拼接完整包: 数据 + CRC + 换行符
    QString fullPayload = dataPart + crcHex + "\n";

    // 5. 发送 (直接调用底层的 write，或者复用 sendMessage)
    // m_socket->write(fullPayload.toUtf8());
    sendMessage(fullPayload);
}

QString BluetoothManager::calculateCRC16(const QByteArray &data)
{
    uint16_t crc = 0xFFFF;

    // 遍历每一个字节
    for (char c : data) {
        // 注意：C++ char 可能是有符号的，进行位运算前必须强转为 uint8_t
        crc ^= static_cast<uint8_t>(c);

        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    // 拆分高低位 (对应 JS: hi.toString(16)... + lo.toString(16)...)
    uint8_t hi = (crc >> 8) & 0xFF;
    uint8_t lo = crc & 0xFF;

    // 格式化为 4位 十六进制字符串 (例如 "A5B6")
    // %02X 表示：16进制，大写，不足2位补0
    return QString::asprintf("%02X%02X", hi, lo);
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
