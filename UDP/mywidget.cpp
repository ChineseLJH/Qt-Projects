#include "mywidget.h"
#include "ui_mywidget.h"
#include <QMessageBox>
#include <QDateTime>
#include <QHostAddress>

myWidget::myWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::myWidget)
    , udpSocket(new QUdpSocket(this))
{
    ui->setupUi(this);


    ui->label_1->setText("模式");
    ui->label_2->setText("IP地址");
    ui->label_3->setText("端口");
    ui->label_4->setText("目标IP");
    ui->label_5->setText("目标端口");

    ui->pushButton_1->setText("打开");
    ui->pushButton_2->setText("发送");

    // 初始化协议选择
    ui->comboBox_1->addItems({"TCP服务端", "TCP客户端", "UDP"});

    // 初始化IP列表
    updateIPList();

    // 连接信号槽
    connect(udpSocket, &QUdpSocket::readyRead, this, &myWidget::onReadyRead);
}

myWidget::~myWidget()
{
    delete ui;
}

void myWidget::on_pushButton_1_clicked()
{
    if(!isConnected){
        // 获取端口号
        bool ok;
        quint16 port = ui->lineEdit_1->text().toUShort(&ok);
        if(!ok || port == 0){
            appendLog("错误：端口号无效");
            return;
        }

        // 尝试绑定端口
        if(udpSocket->bind(port, QUdpSocket::ShareAddress)){
            isConnected = true;
            currentPort = port;
            ui->pushButton_1->setText("停止");
            appendLog(QString("UDP已启动，监听端口：%1").arg(port));
        }else{
            appendLog("错误：端口绑定失败");
        }
    }else{
        udpSocket->close();
        isConnected = false;
        currentPort = 0;
        ui->pushButton_1->setText("启动");
        appendLog("UDP已停止");
    }
}

void myWidget::on_pushButton_2_clicked()
{
    if(!isConnected){
        appendLog("错误：未建立连接");
        return;
    }

    // 获取目标 IP 地址
    QString ipText = ui->comboBox_3->currentText().trimmed();
    QHostAddress targetAddr(ipText);
    if(targetAddr.isNull()){
        appendLog("错误：目标 IP 地址无效");
        return;
    }

    // 获取目标端口号
    bool ok;
    quint16 targetPort = ui->lineEdit_2->text().toUShort(&ok);
    if(!ok || targetPort == 0){
        appendLog("错误：目标端口号无效");
        return;
    }

    // 获取发送内容
    QString sendData = ui->textEdit_2->toPlainText();
    if(sendData.isEmpty()){
        appendLog("警告：发送内容为空");
        return;
    }

    // 发送数据
    qint64 sent = udpSocket->writeDatagram(sendData.toUtf8(), targetAddr, targetPort);
    if(sent == -1){
        appendLog("错误：数据发送失败");
    }else{
        appendLog(QString("已发送 %1 字节到 %2:%3")
                      .arg(sent)
                      .arg(targetAddr.toString())
                      .arg(targetPort));
    }
}

void myWidget::onReadyRead()
{
    while(udpSocket->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress senderAddr;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &senderAddr, &senderPort);

        QString receivedData = QString::fromUtf8(datagram);

        QString ipStr = senderAddr.toString();
        if (senderAddr.protocol() == QAbstractSocket::IPv6Protocol && ipStr.startsWith("::ffff:")) {
            ipStr = ipStr.right(ipStr.length() - 7);  // 去掉 "::ffff:"
        }

        ui->textEdit_1->append(QString("[%1:%2] %3")
                                   .arg(ipStr)
                                   .arg(senderPort)
                                   .arg(receivedData));

        appendLog(QString("收到来自%1:%2的%3字节数据")
                      .arg(ipStr)
                      .arg(senderPort)
                      .arg(datagram.size()));
    }
}

void myWidget::appendLog(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->textEdit_3->append(QString("[%1] %2").arg(timestamp).arg(message));
}

void myWidget::updateIPList()
{
    ui->comboBox_2->clear();
    ui->comboBox_3->clear(); // 清空 comboBox_3

    foreach(const QNetworkInterface &interface, QNetworkInterface::allInterfaces()){
        foreach(const QNetworkAddressEntry &entry, interface.addressEntries()){
            if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol &&
                !entry.ip().isLoopback()){

                QString ipStr = entry.ip().toString();
                ui->comboBox_2->addItem(ipStr); // 本地 IP
                ui->comboBox_3->addItem(ipStr); // 同样加到目标 IP 列表里
            }
        }
    }

    // 可选：设置 comboBox_3 为可编辑，方便手动输入目标 IP
    ui->comboBox_3->setEditable(true);
}
