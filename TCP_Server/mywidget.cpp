#include "mywidget.h"
#include "ui_mywidget.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QDateTime>

myWidget::myWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::myWidget)
    , tcpServer(new QTcpServer(this))
{
    ui->setupUi(this);

    ui->label_1->setText("模式");
    ui->label_2->setText("IP地址");
    ui->label_3->setText("端口");
    ui->label_4->setText("客户端");

    ui->pushButton_1->setText("打开");
    ui->pushButton_2->setText("发送");
    ui->pushButton_3->setText("断开");

    ui->comboBox_1->addItems({"TCP服务端", "TCP客户端", "UDP"});
    updateIPList();

    connect(ui->pushButton_1, &QPushButton::clicked, this, &myWidget::onStartClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &myWidget::onSendClicked);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &myWidget::onDisconnectClicked);

    connect(tcpServer, &QTcpServer::newConnection, this, &myWidget::onNewConnection);
}

myWidget::~myWidget()
{
    delete ui;
}

void myWidget::onStartClicked()
{
    if (!tcpServer->isListening()) {
        // 获取端口和IP
        bool ok;
        quint16 port = ui->lineEdit_1->text().toUShort(&ok);
        QString ip = ui->comboBox_2->currentText();
        QHostAddress address(ip);

        if (!ok || port == 0) {
            appendLog("错误：无效端口");
            return;
        }

        if (!tcpServer->listen(address, port)) {
            appendLog("错误：监听失败");
            return;
        }

        appendLog(QString("服务端启动，监听 %1:%2").arg(ip).arg(port));
        ui->pushButton_1->setText("停止");
    } else {
        foreach (QTcpSocket* s, clients) {
            s->disconnectFromHost();
            s->deleteLater();
        }
        clients.clear();
        tcpServer->close();
        appendLog("服务端已停止");
        ui->pushButton_1->setText("启动");
    }
}

void myWidget::onNewConnection()
{
    while (tcpServer->hasPendingConnections()) {
        QTcpSocket *clientSocket = tcpServer->nextPendingConnection();
        clients.append(clientSocket);
        connect(clientSocket, &QTcpSocket::readyRead, this, [=]() {
            QByteArray data = clientSocket->readAll();
            QString str = QString::fromUtf8(data);
            QString from = clientSocket->peerAddress().toString();
            quint16 port = clientSocket->peerPort();
            ui->textEdit_1->append(QString("[%1:%2] %3").arg(from).arg(port).arg(str));
            appendLog(QString("收到 %1:%2 的 %3 字节数据").arg(from).arg(port).arg(data.size()));
        });
        connect(clientSocket, &QTcpSocket::disconnected, this, [=]() {
            clients.removeAll(clientSocket);
            appendLog(QString("客户端 %1:%2 断开连接")
                          .arg(clientSocket->peerAddress().toString())
                          .arg(clientSocket->peerPort()));
            clientSocket->deleteLater();
        });
        appendLog(QString("新连接来自 %1:%2")
                      .arg(clientSocket->peerAddress().toString())
                      .arg(clientSocket->peerPort()));
        updateClientList();
    }
}

void myWidget::onSendClicked()
{
    QString message = ui->textEdit_2->toPlainText();
    if (message.isEmpty()) {
        appendLog("警告：发送内容为空");
        return;
    }

    QByteArray data = message.toUtf8();
    QString target = ui->comboBox_3->currentText();

    if (target == "All") {
        for (QTcpSocket *client : clients) {
            if (client->state() == QAbstractSocket::ConnectedState) {
                client->write(data);
            }
        }
        appendLog(QString("向所有客户端发送 %1 字节数据").arg(data.size()));
    } else {
        for (QTcpSocket *client : clients) {
            QString identifier = QString("%1:%2")
            .arg(client->peerAddress().toString())
                .arg(client->peerPort());
            if (identifier == target && client->state() == QAbstractSocket::ConnectedState) {
                client->write(data);
                appendLog(QString("向 %1 发送 %2 字节数据").arg(identifier).arg(data.size()));
                break;
            }
        }
    }
}

void myWidget::appendLog(const QString &msg)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->textEdit_3->append(QString("[%1] %2").arg(time, msg));
}

void myWidget::updateIPList()
{
    ui->comboBox_2->clear();
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            QHostAddress ip = entry.ip();
            if (ip.protocol() == QAbstractSocket::IPv4Protocol && !ip.isLoopback()) {
                ui->comboBox_2->addItem(ip.toString());
            }
        }
    }
    ui->comboBox_2->setEditable(true); // 支持手动输入
}

void myWidget::updateClientList()
{
    ui->comboBox_3->clear();
    ui->comboBox_3->addItem("All");

    for (QTcpSocket* client : clients) {
        QString identifier = QString("%1:%2")
        .arg(client->peerAddress().toString())
            .arg(client->peerPort());
        ui->comboBox_3->addItem(identifier);
    }
}

void myWidget::onDisconnectClicked()
{
    QString target = ui->comboBox_3->currentText();

    if (target == "All") {
        for (QTcpSocket* client : clients) {
            client->disconnectFromHost();
        }
        appendLog("已断开所有客户端连接");
    } else {
        for (QTcpSocket* client : clients) {
            QString identifier = QString("%1:%2")
            .arg(client->peerAddress().toString())
                .arg(client->peerPort());
            if (identifier == target) {
                client->disconnectFromHost();
                appendLog(QString("已断开客户端 %1").arg(identifier));
                break;
            }
        }
    }
    updateClientList();
}

