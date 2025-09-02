#include "mywidget.h"
#include "ui_mywidget.h"
#include <QMessageBox>
#include <QDateTime>
#include <QHostAddress>
#include <QTcpSocket>  // 引入 QTcpSocket

myWidget::myWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::myWidget)
    , tcpSocket(new QTcpSocket(this))  // 使用 QTcpSocket
{
    ui->setupUi(this);

    ui->label_1->setText("模式");
    ui->label_2->setText("目标IP");
    ui->label_3->setText("目标端口");

    ui->pushButton_1->setText("连接");
    ui->pushButton_2->setText("发送");

    // 初始化协议选择
    ui->comboBox_1->addItems({"TCP服务端", "TCP客户端", "UDP"});

    // 初始化IP列表
    updateIPList();

    // 连接信号槽
    connect(tcpSocket, &QTcpSocket::readyRead, this, &myWidget::onReadyRead);
    connect(tcpSocket, &QTcpSocket::connected, this, &myWidget::onConnected);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &myWidget::onDisconnected);
}

myWidget::~myWidget()
{
    delete ui;
}

void myWidget::on_pushButton_1_clicked()
{
//    !tcpSocket->isOpen()
    if(tcpSocket->state() != QAbstractSocket::ConnectedState){
        // 获取目标 IP 地址
        QString ipText = ui->comboBox_2->currentText().trimmed();
        QHostAddress targetAddr(ipText);
        if(targetAddr.isNull()){
            appendLog("错误：目标 IP 地址无效");
            return;
        }

        // 获取目标端口号
        bool ok;
        quint16 targetPort = ui->lineEdit_1->text().toUShort(&ok);
        if(!ok || targetPort == 0){
            appendLog("错误：目标端口号无效");
            return;
        }

        // 连接到目标服务端
        tcpSocket->connectToHost(targetAddr, targetPort);
        if (!tcpSocket->waitForConnected(1000)) {  // 连接超时为5秒
            appendLog("错误：连接失败");
        } else {
            appendLog(QString("已连接到 %1:%2").arg(targetAddr.toString()).arg(targetPort));
            ui->pushButton_1->setText("断开");
        }
    } else {
        // 如果已经连接，点击按钮断开连接
        tcpSocket->disconnectFromHost();
        // tcpSocket->abort();
        ui->pushButton_1->setText("连接");
        appendLog("已断开连接");
    }
}

void myWidget::on_pushButton_2_clicked()
{
    if(!tcpSocket->isOpen()){
        appendLog("错误：未建立连接");
        return;
    }

    // 获取发送内容
    QString sendData = ui->textEdit_2->toPlainText();
    if(sendData.isEmpty()){
        appendLog("警告：发送内容为空");
        return;
    }

    // 发送数据
    qint64 sent = tcpSocket->write(sendData.toUtf8());
    if(sent == -1){
        appendLog("错误：数据发送失败");
    } else {
        appendLog(QString("已发送 %1 字节数据").arg(sent));
    }
}

void myWidget::onReadyRead()
{
    // 读取接收到的数据
    QByteArray receivedData = tcpSocket->readAll();
    QString receivedStr = QString::fromUtf8(receivedData);
    ui->textEdit_1->append(QString("[%1:%2] %3")
                               .arg(tcpSocket->peerAddress().toString())
                               .arg(tcpSocket->peerPort())
                               .arg(receivedStr));

    appendLog(QString("收到来自 %1:%2 的数据: %3")
                  .arg(tcpSocket->peerAddress().toString())
                  .arg(tcpSocket->peerPort())
                  .arg(receivedStr));
}

void myWidget::onConnected()
{
    appendLog("连接成功！");
}

void myWidget::onDisconnected()
{
    appendLog("连接断开");
    tcpSocket->disconnectFromHost();
    // tcpSocket->abort();
    ui->pushButton_1->setText("连接");
}

void myWidget::appendLog(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->textEdit_3->append(QString("[%1] %2").arg(timestamp).arg(message));
}

void myWidget::updateIPList()
{
    ui->comboBox_2->clear();

    foreach(const QNetworkInterface &interface, QNetworkInterface::allInterfaces()){
        foreach(const QNetworkAddressEntry &entry, interface.addressEntries()){
            if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol &&
                !entry.ip().isLoopback()){
                QString ipStr = entry.ip().toString();
                ui->comboBox_2->addItem(ipStr); // 本地 IP
            }
        }
    }

    ui->comboBox_2->setEditable(true);
}
