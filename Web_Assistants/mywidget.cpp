#include "mywidget.h"
#include "ui_mywidget.h"

myWidget::myWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::myWidget)
{
    ui->setupUi(this);

    ui->label_1->setText("模式选择");
    ui->label_2->setText("本地IP");
    ui->label_3->setText("本地端口");
    ui->label_4->setText("目标IP");
    ui->label_5->setText("目标端口");
    ui->label_6->setText("客户端");

    ui->pushButton_TCP_Client->setText("TCP Client");
    ui->pushButton_TCP_Server->setText("TCP Server");
    ui->pushButton_UDP->setText("UDP");

    ui->pushButton_1->setText("连接");
    ui->pushButton_2->setText("发送");
    ui->pushButton_3->setText("断开");

    ui->comboBox_1->setEditable(false); // 本地 IP（服务端/UDP）
    ui->comboBox_2->setEditable(true);  // 目标 IP（客户端/UDP）
    ui->comboBox_3->setEditable(false); // TCP 客户端列表

    updateLocalIPList(ui->comboBox_1);
    updateLocalIPList(ui->comboBox_2);

}

void myWidget::appendLog(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->textEdit_3->append(QString("[%1] %2").arg(timestamp).arg(message));
}

void myWidget::on_pushButton_TCP_Client_clicked()
{
    mode=1;

    ui->label_4->show();
    ui->label_5->show();
    ui->comboBox_2->show();
    ui->lineEdit_2->show();

    ui->label_2->hide();
    ui->comboBox_1->hide();
    ui->label_3->hide();
    ui->lineEdit_1->hide();
    ui->widget_3->hide();

    if(state) closeAllConnections();

    appendLog("已切换至TCP_Client");
}

void myWidget::on_pushButton_TCP_Server_clicked()
{
    mode=2;

    ui->label_2->show();
    ui->comboBox_1->show();
    ui->label_3->show();
    ui->lineEdit_1->show();
    ui->widget_3->show();

    ui->label_4->hide();
    ui->label_5->hide();
    ui->comboBox_2->hide();
    ui->lineEdit_2->hide();

    if(state) closeAllConnections();

    appendLog("已切换至TCP_Server");
}

void myWidget::on_pushButton_UDP_clicked()
{
    mode=3;

    ui->label_2->show();
    ui->comboBox_1->show();
    ui->label_3->show();
    ui->lineEdit_1->show();
    ui->label_4->show();
    ui->label_5->show();
    ui->comboBox_2->show();
    ui->lineEdit_2->show();

    ui->widget_3->hide();

    if(state) closeAllConnections();

    appendLog("已切换至UDP");
}


myWidget::~myWidget()
{
    delete ui;
}

void myWidget::updateLocalIPList(QComboBox *comboBox)
{
    comboBox->clear();
    const auto &interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            QHostAddress ip = entry.ip();
            if (ip.protocol() == QAbstractSocket::IPv4Protocol && !ip.isLoopback()) {
                comboBox->addItem(ip.toString());
            }
        }
    }
}

void myWidget::updateTcpClientList()
{
    ui->comboBox_3->clear();
    ui->comboBox_3->addItem("All");
    for (QTcpSocket *client : tcpClients) {
        QString ipPort = QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
        ui->comboBox_3->addItem(ipPort);
    }
}

QTcpSocket* myWidget::getSelectedTcpClient()
{
    QString selected = ui->comboBox_3->currentText();
    for (QTcpSocket *client : tcpClients) {
        QString ipPort = QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
        if (selected == ipPort) return client;
    }
    return nullptr;
}

void myWidget::updateLocalIPList()
{
    ui->comboBox_1->clear();
    ui->comboBox_2->clear();

    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            QHostAddress ip = entry.ip();
            if (ip.protocol() == QAbstractSocket::IPv4Protocol && !ip.isLoopback()) {
                ui->comboBox_1->addItem(ip.toString());
                ui->comboBox_2->addItem(ip.toString()); // 目标IP可选
            }
        }
    }
    ui->comboBox_2->setEditable(true); // 支持手动输入目标IP
}

void myWidget::updateClientList()
{
    ui->comboBox_3->clear();
    ui->comboBox_3->addItem("All");

    for (QTcpSocket *client : tcpClients) {
        QString ipPort = QString("%1:%2")
        .arg(client->peerAddress().toString())
            .arg(client->peerPort());
        ui->comboBox_3->addItem(ipPort);
    }
}

void myWidget::on_pushButton_1_clicked()
{
    if(state)
    {
        closeAllConnections();
        ui->pushButton_1->setText("连接");
        state=false;
        return;
    }
    if (mode == 0) {
        appendLog("请先选择工作模式！");
        return;
    }

    if (mode == 1) { // TCP 客户端
        QString ip = ui->comboBox_2->currentText();
        quint16 port = ui->lineEdit_2->text().toUShort();

        if (tcpSocket) {
            tcpSocket->abort();
            delete tcpSocket;
            tcpSocket = nullptr;
        }

        tcpSocket = new QTcpSocket(this);

        // 连接成功
        connect(tcpSocket, &QTcpSocket::connected, this, [=]() {
            appendLog("连接成功！");
            ui->pushButton_1->setText("断开");
            state=true;
        });

        // 接收到数据
        connect(tcpSocket, &QTcpSocket::readyRead, this, [=]() {
            QByteArray data = tcpSocket->readAll();
            QString msg = QString("[%1:%2] %3")
                              .arg(ip)
                              .arg(port)
                              .arg(QString::fromUtf8(data));
            ui->textEdit_1->append(msg);
        });

        // 连接断开
        connect(tcpSocket, &QTcpSocket::disconnected, this, [=]() {
            if(state) appendLog("服务器断开连接！");
            state = false;
            ui->pushButton_1->setText("连接");
        });

        // 连接失败
        connect(tcpSocket, &QTcpSocket::errorOccurred, this, [=](QAbstractSocket::SocketError socketError) {
            QString errorMsg;
            switch (socketError) {
            case QAbstractSocket::ConnectionRefusedError:
                errorMsg = "连接被拒绝，请检查服务器是否启动，IP 地址和端口是否正确。";
                break;
            case QAbstractSocket::HostNotFoundError:
                errorMsg = "找不到主机，检查 IP 地址是否正确。";
                break;
            case QAbstractSocket::RemoteHostClosedError:
                errorMsg = "远程主机关闭连接。";
                break;
            default:
                errorMsg = tcpSocket->errorString();
                break;
            }
            appendLog(QString("连接失败: %1").arg(errorMsg));
        });

        // 尝试连接
        tcpSocket->connectToHost(ip, port);
        appendLog(QString("正在连接 %1:%2...").arg(ip).arg(port));
    }


    else if (mode == 2) { // TCP 服务端
        QString ip = ui->comboBox_1->currentText();
        quint16 port = ui->lineEdit_1->text().toUShort();

        if (tcpServer) {
            tcpServer->close();
            delete tcpServer;
            tcpServer = nullptr;
        }

        tcpServer = new QTcpServer(this);
        connect(tcpServer, &QTcpServer::newConnection, this, [=]() {
            QTcpSocket *client = tcpServer->nextPendingConnection();
            tcpClients.append(client);

            appendLog(QString("客户端连接：%1:%2")
                          .arg(client->peerAddress().toString())
                          .arg(client->peerPort()));

            updateClientList();

            connect(client, &QTcpSocket::readyRead, this, [=]() {
                QByteArray data = client->readAll();
                QString msg = QString("[%1:%2] %3")
                                  .arg(client->peerAddress().toString())
                                  .arg(client->peerPort())
                                  .arg(QString::fromUtf8(data));
                ui->textEdit_1->append(msg);
            });

            connect(client, &QTcpSocket::disconnected, this, [=]() {
                appendLog(QString("客户端断开：%1:%2")
                              .arg(client->peerAddress().toString())
                              .arg(client->peerPort()));
                tcpClients.removeAll(client);
                client->deleteLater();
                updateClientList();
            });
        });

        if (!tcpServer->listen(QHostAddress(ip), port)) {
            appendLog("监听失败！");
            return;
        }

        appendLog(QString("开始监听：%1:%2").arg(ip).arg(port));
        ui->pushButton_1->setText("断开");
        state=true;
    }

    else if (mode == 3) { // UDP
        QString localIp = ui->comboBox_1->currentText();
        quint16 localPort = ui->lineEdit_1->text().toUShort();

        if (udpSocket) {
            udpSocket->close();
            delete udpSocket;
            udpSocket = nullptr;
        }

        udpSocket = new QUdpSocket(this);
        bool success = udpSocket->bind(QHostAddress(localIp), localPort);
        if (success) {
            appendLog(QString("已绑定 UDP %1:%2").arg(localIp).arg(localPort));
            ui->pushButton_1->setText("断开");
            state=true;
        } else {
            appendLog("绑定失败！");
            return;
        }

        connect(udpSocket, &QUdpSocket::readyRead, this, [=]() {
            while (udpSocket->hasPendingDatagrams()) {
                QByteArray datagram;
                datagram.resize(udpSocket->pendingDatagramSize());
                QHostAddress sender;
                quint16 senderPort;
                udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
                ui->textEdit_1->append(
                    QString("[%1:%2] %3")
                        .arg(sender.toString())
                        .arg(senderPort)
                        .arg(QString::fromUtf8(datagram)));
            }
        });
    }

    // ui->pushButton_1->setText("断开");
    // state=true;
}

void myWidget::on_pushButton_2_clicked()
{
    if (mode == 0) {
        appendLog("请先选择工作模式！");
        return;
    }

    if (ui->textEdit_2->toPlainText().isEmpty()) {
        appendLog("发送内容为空！");
        return;
    }

    if (mode == 1) { // TCP 客户端
        if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
            QString message = ui->textEdit_2->toPlainText();
            tcpSocket->write(message.toUtf8());
            appendLog("已发送");
        } else {
            appendLog("未连接到服务器！");
        }
    }


    QString message = ui->textEdit_2->toPlainText();

    if (mode == 2) { // TCP 服务端
        QString target = ui->comboBox_3->currentText();

        if (target == "All") {
            for (QTcpSocket *client : tcpClients) {
                client->write(message.toUtf8());
            }
            appendLog("已群发消息");
        } else {
            QTcpSocket *client = findClientByAddress(target);
            if (client) {
                client->write(message.toUtf8());
                appendLog("已发送给：" + target);
            } else {
                appendLog("未找到指定客户端！");
            }
        }
    }

    else if (mode == 3) { // UDP
        if (!udpSocket) {
            appendLog("请先绑定端口");
            return;
        }

        QString targetIp = ui->comboBox_2->currentText();
        quint16 targetPort = ui->lineEdit_2->text().toUShort();
        QString message = ui->textEdit_2->toPlainText();

        udpSocket->writeDatagram(message.toUtf8(), QHostAddress(targetIp), targetPort);
        appendLog(QString("已发送到 %1:%2").arg(targetIp).arg(targetPort));
    }


    // 其他模式的发送将在下一部分中提供
}

void myWidget::on_pushButton_3_clicked()
{
    if (mode == 0) {
        appendLog("请先选择工作模式！");
        return;
    }

    if (mode == 1) { // TCP 客户端
        if (tcpSocket) {
            tcpSocket->disconnectFromHost();
            appendLog("客户端断开连接");
        }
    }

    if (mode == 2) { // TCP 服务端
        QString target = ui->comboBox_3->currentText();

        if (target == "All") {
            for (QTcpSocket *client : tcpClients) {
                client->disconnectFromHost();
            }
            appendLog("已断开所有客户端");
        } else {
            QTcpSocket *client = findClientByAddress(target);
            if (client) {
                client->disconnectFromHost();
                appendLog("已断开客户端：" + target);
            } else {
                appendLog("未找到指定客户端！");
            }
        }
    }

    else if (mode == 3) {
        if (udpSocket) {
            udpSocket->close();
            appendLog("UDP 已断开");
        }
    }


    // 其他模式的断开将在后面继续补充
}

QTcpSocket* myWidget::findClientByAddress(const QString &address)
{
    for (QTcpSocket *client : tcpClients) {
        QString ipPort = QString("%1:%2")
        .arg(client->peerAddress().toString())
            .arg(client->peerPort());
        if (ipPort == address) {
            return client;
        }
    }
    return nullptr;
}

void myWidget::closeAllConnections()
{
    state = false;

    if (tcpSocket) {
        if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
            tcpSocket->disconnectFromHost();
            tcpSocket->waitForDisconnected(1000); // 最多等1秒
        }
        tcpSocket->deleteLater();
        tcpSocket = nullptr;
    }

    // 断开 TCP 服务端所有客户端连接
    for (QTcpSocket *clientSocket : tcpClients) {
        if (clientSocket->state() == QAbstractSocket::ConnectedState) {
            clientSocket->disconnectFromHost();
            clientSocket->waitForDisconnected(1000);
        }
        clientSocket->deleteLater();
    }
    tcpClients.clear();

    // 关闭 TCP 服务器监听
    if (tcpServer) {
        tcpServer->close();
        tcpServer->deleteLater();
        tcpServer = nullptr;
    }

    // 关闭 UDP Socket
    if (udpSocket) {
        if (udpSocket->state() == QAbstractSocket::BoundState) {
            udpSocket->close();
        }
        udpSocket->deleteLater();
        udpSocket = nullptr;
    }

    // 清空客户端列表下拉框
    ui->comboBox_3->clear();

    // 清除其他状态

    appendLog("已断开所有连接");

}
