#include "mywidget.h"
#include "ui_mywidget.h"

myWidget::myWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::myWidget)
{
    ui->setupUi(this);

    ui->LBSerialPort_1->setText("串口");
    ui->LBSerialPort_2->setText("波特率");
    ui->LBSerialPort_3->setText("校验位");
    ui->LBSerialPort_4->setText("数据位");
    ui->LBSerialPort_5->setText("停止位");

    // **动态检测可用串口**
    updateSerialPortList();

    // 设置可选参数
    ui->CboxSerialPort_2->addItems({"9600", "115200"});
    ui->CboxSerialPort_3->addItems({"None", "Even", "Odd"});
    ui->CboxSerialPort_4->addItems({"5", "6", "7", "8"});
    ui->CboxSerialPort_5->addItems({"1", "1.5", "2"});

    ui->pushButton_1->setText("打开");
    ui->pushButton_2->setText("发送");
    ui->pushButton_3->setText("清空接受");
    ui->pushButton_4->setText("更新");
    ui->pushButton_5->setText("改为深色");

    ui->checkBox_1->setText("16进制显示");
    ui->checkBox_2->setText("16进制发送");

    connect(&mSerialPort, &QSerialPort::readyRead, this, &myWidget::readSerialData);

    connect(ui->pushButton_1, &QPushButton::clicked,this, &myWidget::openSerialPort);
    connect(ui->pushButton_2, &QPushButton::clicked,this, &myWidget::sendData);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &myWidget::clearReceivedData);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &myWidget::updateSerialPortList);

    this->setStyleSheet("background-color: #F5F5F5; color: black;"); // 整个窗口
    ui->textEdit_1->setStyleSheet("background-color: #FAFAFA; color: black; border: 1px solid #CCCCCC;");
    ui->textEdit_2->setStyleSheet("background-color: #FAFAFA; color: black; border: 1px solid #CCCCCC;");
    ui->textEdit_3->setStyleSheet("background-color: #FAFAFA; color: black; border: 1px solid #CCCCCC;");

}

myWidget::~myWidget()
{
    if (mSerialPort.isOpen()) {
        mSerialPort.close();
    }
    delete ui;
}

// **刷新可用串口列表**
void myWidget::updateSerialPortList()
{
    appendMessage("\n");

    ui->CboxSerialPort_1->clear();

    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    if (availablePorts.isEmpty()) {
        appendMessage("⚠ 没有检测到可用的串口！");
        return;
    }

    for (const QSerialPortInfo &port : availablePorts) {
        QString portInfo = QString(" %1: %2\n")
                               .arg(port.portName())
                               .arg(port.description().isEmpty() ? "未知" : port.description());

        ui->CboxSerialPort_1->addItem(port.portName()); // 添加到下拉框
        appendMessage(portInfo); // 在日志里显示串口信息
    }
    appendMessage("✅ 可用串口已更新");
}

// **切换串口**
void myWidget::on_CboxSerialPort_1_currentTextChanged(const QString &arg1)
{
    if (mSerialPort.portName() == arg1) {
        return;
    }

    if (mSerialPort.isOpen()) {
        mSerialPort.close();
    }

    mSerialPort.setPortName(arg1);
}

// **打开串口**
void myWidget::openSerialPort()
{
    if (mSerialPort.isOpen()) {
        mSerialPort.close();
        ui->pushButton_1->setText("打开");
        appendMessage("🔌 串口已关闭");
        return;
    }

    // **获取 UI 参数**
    QString portName = ui->CboxSerialPort_1->currentText();
    int baudRate = ui->CboxSerialPort_2->currentText().toInt();
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;

    if (ui->CboxSerialPort_3->currentText() == "Even") {
        parity = QSerialPort::EvenParity;
    } else if (ui->CboxSerialPort_3->currentText() == "Odd") {
        parity = QSerialPort::OddParity;
    }

    dataBits = static_cast<QSerialPort::DataBits>(ui->CboxSerialPort_4->currentText().toInt());

    if (ui->CboxSerialPort_5->currentText() == "1.5") {
        stopBits = QSerialPort::OneAndHalfStop;
    } else if (ui->CboxSerialPort_5->currentText() == "2") {
        stopBits = QSerialPort::TwoStop;
    }

    // **设置串口参数**
    mSerialPort.setPortName(portName);
    mSerialPort.setBaudRate(baudRate);
    mSerialPort.setParity(parity);
    mSerialPort.setDataBits(dataBits);
    mSerialPort.setStopBits(stopBits);

    if (mSerialPort.open(QSerialPort::ReadWrite)) {
        appendMessage("✅ 串口 " + portName + " 打开成功");
        ui->pushButton_1->setText("关闭");
    } else {
        appendMessage("❌ 串口打开失败：" + mSerialPort.errorString());
    }
}

// **发送数据**
void myWidget::sendData()
{
    if (!mSerialPort.isOpen()) {
        appendMessage("❌ 串口未打开，无法发送数据");
        return;
    }

    QString dataToSend = ui->textEdit_2->toPlainText();
    if (dataToSend.isEmpty()) {
        appendMessage("⚠️ 发送内容为空");
        return;
    }

    QByteArray byteArray = dataToSend.toUtf8();
    mSerialPort.write(byteArray);
    appendMessage("📤 发送数据：" + dataToSend);
}

void myWidget::readSerialData()
{
    QByteArray data = mSerialPort.readAll(); // 读取串口数据
    if (data.isEmpty()) return;

    QString displayText;
    if (displayHex_1) {
        displayText = data.toHex(' ').toUpper(); // 16进制显示（空格分隔）
    } else {
        displayText = QString::fromUtf8(data); // 正常字符串显示
    }

    // 在 textEdit_1 追加显示数据
    ui->textEdit_1->append(displayText);
}

void myWidget::on_checkBox_1_toggled(bool checked)
{

    displayHex_1 = checked; // 勾选时，设置为 true
    if(displayHex_1) {
        appendMessage("⚠️ 16进制输入已打开");
    } else {
        appendMessage("⚠️ 16进制输入已关闭");
    }
}

void myWidget::on_checkBox_2_toggled(bool checked)
{

    displayHex_2 = checked; // 勾选时，设置为 true
    if(displayHex_2) {
        appendMessage("⚠️ 16进制输出已打开");
    } else {
        appendMessage("⚠️ 16进制输出已关闭");
    }
}

void myWidget::appendMessage(const QString &message)
{
    ui->textEdit_3->append(message); // 在 textEdit_3 追加信息
}

void myWidget::clearReceivedData()
{
    ui->textEdit_1->clear();  // 清空接收区
    appendMessage("🗑 接收数据已清空");  // 在 textEdit_3 显示日志
}

void myWidget::on_pushButton_5_clicked()
{
    static bool isDarkMode = false; // 记录当前模式

    if (isDarkMode) {
        // 切换到 **浅色模式**
        this->setStyleSheet("background-color: #F5F5F5; color: black;"); // 整个窗口
        ui->textEdit_1->setStyleSheet("background-color: #FAFAFA; color: black; border: 1px solid #CCCCCC;");
        ui->textEdit_2->setStyleSheet("background-color: #FAFAFA; color: black; border: 1px solid #CCCCCC;");
        ui->textEdit_3->setStyleSheet("background-color: #FAFAFA; color: black; border: 1px solid #CCCCCC;");
        ui->pushButton_5->setText("改为深色");
    } else {
        // 切换到 **深色模式**
        this->setStyleSheet("background-color: #2E2E2E; color: white;"); // 整个窗口
        ui->textEdit_1->setStyleSheet("background-color: #1E1E1E; color: #DCDCDC; border: 1px solid #555555;");
        ui->textEdit_2->setStyleSheet("background-color: #1E1E1E; color: #DCDCDC; border: 1px solid #555555;");
        ui->textEdit_3->setStyleSheet("background-color: #1E1E1E; color: #DCDCDC; border: 1px solid #555555;");
        ui->pushButton_5->setText("改为浅色");
    }

    isDarkMode = !isDarkMode; // 切换模式状态
}
