#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include "qbluetoothlocaldevice.h"
#include <QObject>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QBluetoothSocket>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothServiceInfo>

// #include <QtDBus/QDBusInterface>  // 修复 'QDBusInterface' 未声明错误
// #include <QtDBus/QDBusConnection> // 修复 'QDBusConnection' 未声明错误

class BluetoothManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString targetAddress READ targetAddress WRITE setTargetAddress NOTIFY targetAddressChanged) // 新增
public:
    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager();

    Q_INVOKABLE void startScan();                                            // 扫描设备
    Q_INVOKABLE void connectToDevice(const QString &address, const QString &uuidStr); // 连接指定地址与信道
    Q_INVOKABLE void disconnect();                                           // 断开连接
    Q_INVOKABLE void sendMessage(const QString &msg);                        // 发送消息

    // Q_INVOKABLE void pairDevice(const QString &address, const QString &password); // 声明接口
    Q_INVOKABLE void registerPairingAgent(); // 声明接口

    QString targetAddress() const;        // Getter
    void setTargetAddress(const QString &address); // Setter

signals:
    void deviceDiscovered(const QString &name, const QString &address);
    void connected();
    void disconnected();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    // void registerPairingAgent(const QString &password);
    void paired();  // 新增配对成功信号

    void targetAddressChanged();          // 新增信号
    void pairingPinRequested(const QString &deviceName);

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(const QBluetoothSocket::SocketError &err);

private:
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent = nullptr;
    QBluetoothSocket *m_socket = nullptr;
    QBluetoothLocalDevice *m_localDevice = nullptr; // 新增成员变量

    QString m_targetAddress;              // 成员变量
};

#endif // BLUETOOTHMANAGER_H

