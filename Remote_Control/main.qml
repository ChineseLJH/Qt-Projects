import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import MyApp 1.0    // UdpDataClient, MyUdp
import "."
// Joystick.qml

ApplicationWindow {
    id: appWin
    width: 800;
    height: 480
    visible: true
    title: qsTr("遥控器")
    flags: Qt.Window | Qt.CustomizeWindowHint

    property string targetIp: ""

    MyUdp {
        id: udp
        onIpReceived: function(ip) {
            console.log("发现设备 IP:", ip)
            targetIp = ip
        }
    }

    Component.onCompleted: {
        udp.startListening(54321)  // 监听 ESP32 的广播
    }

    // —— 状态属性 —— //
    property real stickAngle: 0
    property real stickDistance: 0
    property var buttonStates: {
        "前倾": false,
        "后仰": false,
        "击球": false,
        "备用1": false
    }

    // —— 计数器 —— //
    property var txHistory: []
    property var rxHistory: []
    property real currentLossRate: 0.0
    property bool hasEnoughData: false

    // 【核心修改 1】实例化新的底层 UDP 数据发送类
    UdpDataClient { id: udpData }

    // —— 接收信号 —— //
    Connections {
        target: udpData
        onDataReceived: function(data) {
            if (udpData.connected) {
                rxHistory.push(Date.now())
            }
        }
    }


    // —— CRC-16校验计算 —— //
    function calculateCRC(str) {
            let crc = 0xFFFF;
            for (let i = 0; i < str.length; i++) {
                crc ^= str.charCodeAt(i);
                for (let j = 0; j < 8; j++) {
                    const bit = crc & 0x0001;
                    crc >>= 1;
                    if (bit) crc ^= 0xA001; //0xA001 是 0x8005 的 位序反转形式   若采用 右移计算方式，需使用反转后的多项式 0xA001。
                }
            }
            //MODBUS 等协议要求 CRC 以 小端模式（Little-Endian） 传输，即低字节在前、高字节在后
            const lo = crc & 0xFF;
            const hi = (crc >> 8) & 0xFF;
            return  hi.toString(16).padStart(2, '0').toUpperCase()+
                    lo.toString(16).padStart(2, '0').toUpperCase();
    }


    // 增加一个私有变量用于记录静止帧数
    property int idleFrameCount: 0

    Timer {
        interval: 20; // 维持 50Hz 采样率
        running: true; repeat: true
        onTriggered: {
            if (udpData.connected) {
                // 1. 判断是否处于物理静止态 (摇杆归零且按键全部松开)
                let isIdle = (stickAngle === 0 && stickDistance === 0 &&
                                              !buttonStates["前倾"] && !buttonStates["后仰"] &&
                                              !buttonStates["击球"] && !buttonStates["备用1"] &&
                                              !buttonStates["A"] && !buttonStates["B"] &&
                                              !buttonStates["C"] && !buttonStates["D"]);

                if (isIdle) {
                    idleFrameCount++;
                    // 如果连续静止超过 100ms (5 帧)，则每 5 帧只发 1 帧 (降频至 10Hz)
                    // 这既能维持心跳检测，又极大清空了信道占用
                    if (idleFrameCount > 5 && (idleFrameCount % 5 !== 0)) {
                        return;
                    }
                } else {
                    idleFrameCount = 0; // 一旦有动作，立刻恢复 50Hz 实时性
                }

                const a = buttonStates["前倾"] ? 1 : 0
                const b = buttonStates["后仰"] ? 1 : 0
                const c = buttonStates["击球"] ? 1 : 0
                const d = buttonStates["备用1"] ? 1 : 0

                const btnA = buttonStates["A"] ? 1 : 0
                const btnB = buttonStates["B"] ? 1 : 0
                const btnC = buttonStates["C"] ? 1 : 0
                const btnD = buttonStates["D"] ? 1 : 0

                // 将数据格式扩展，拼入 ABCD 的状态
                const dataPart = `[${stickAngle.toFixed(1)};${stickDistance.toFixed(2)};${a};${b};${c};${d};${btnA};${btnB};${btnC};${btnD}]`
                const crc = calculateCRC(dataPart)
                const payload = `${dataPart}${crc}\n`

                udpData.sendMessage(payload)
                txHistory.push(Date.now())
            }
        }
    }

    Timer {
            interval: 200
            running: true
            repeat: true
            onTriggered: {
                const now = Date.now()
                const cutoff = now - 10000 // 严格界定 10 秒（10000 毫秒）的积分下限

                // 1. 内存裁剪（O(n) 复杂度）：剔除时间窗口之外的过期元素
                while (txHistory.length > 0 && txHistory[0] < cutoff) {
                    txHistory.shift()
                }
                while (rxHistory.length > 0 && rxHistory[0] < cutoff) {
                    rxHistory.shift()
                }

                // 2. 统计计算：需要累计至少 1 秒的数据（约 50 个发包）才显示结果，避免初始数据毛刺
                if (txHistory.length < 50) {
                    hasEnoughData = false
                } else {
                    hasEnoughData = true
                    // 乘法补偿：因为下位机配置为 10 个包触发一次 ACK
                    // 预期收到 ACK 数量 = txHistory.length / 10
                    let estReceived = rxHistory.length * 10

                    let rate = 1.0 - (estReceived / txHistory.length)

                    // 边界钳位：防止因 10:1 量化误差导致的临时负溢出
                    if (rate < 0) rate = 0.0
                    currentLossRate = rate * 100
                }
            }
        }

    Row {
        anchors.fill: parent
        anchors.margins: 40
        spacing: parent.width * 0.05  // 三部分之间自动调整间距

        // 左侧 Joystick 容器
        Item {
            id: joystickContainer
            width: parent.width * 0.25
            height: width
            anchors.verticalCenter: parent.verticalCenter

            Joystick {
                anchors.fill: parent
                onChanged: {
                    stickAngle = angle
                    stickDistance = distance
                }
            }
        }

        // 中间信息区容器
        Item {
            id: centerContainer
            width: parent.width * 0.3
            anchors.verticalCenter: parent.verticalCenter
            height: joystickContainer.height

            Column {
                anchors.centerIn: parent
                spacing: 12

                Text { text: "角度: " + stickAngle.toFixed(1) + "°"; font.pixelSize: 24 }
                Text { text: "速度: " + stickDistance.toFixed(3); font.pixelSize: 24 }
                Text {
                    // 使用 JavaScript 的闭包/代码块来进行严谨的状态机判断
                    text: {
                        if (!udpData.connected) {
                            // 状态 1：底层 UDP Socket 未连接或已手动断开，停止发送
                            return "丢包: N/A"
                        } else if (!hasEnoughData) {
                            // 状态 2：Socket 已连接正在发送，但 txHistory 数组长度小于设定的 50 个下限
                            return "丢包: 采样缓冲中..."
                        } else {
                            // 状态 3：数组数据饱满，执行浮点数渲染
                            return "丢包: " + currentLossRate.toFixed(2) + "%"
                        }
                    }
                    font.pixelSize: 24
                }

                Button {
                    width: 160; height: 48
                    text: udpData.connected ? qsTr("禁用控制") : qsTr("使能控制") // 【核心修改 5】更正语义，因为没有物理连接过程
                    onClicked: {
                        if (udpData.connected)
                            udpData.disconnectFromHost() // 停止发送
                        else if (targetIp !== "")
                            udpData.connectToHost(targetIp, 6666) // 【核心修改 6】更改底层目标端口为 6666
                        else
                            console.log("还未收到设备 IP，无法使能发送")
                    }
                }
            }
        }

        // 右侧按钮容器
        // 右侧按钮容器
                Item {
                    id: buttonPanel
                    width: parent.width * 0.25
                    anchors.verticalCenter: parent.verticalCenter
                    height: joystickContainer.height

                    // 提取公共按钮组件，避免代码重复
                    Component {
                        id: controlButton
                        Button {
                            text: modelData
                            width: 80; height: 48

                            onPressedChanged: {
                                buttonStates[modelData] = pressed
                            }
                            onCanceled: {
                                buttonStates[modelData] = false
                            }

                            background: Rectangle {
                                color: parent.pressed ? "#4CAF50" : "#E0E0E0"
                                radius: 4
                            }

                            contentItem: Text {
                                text: parent.text
                                color: parent.pressed ? "white" : "black"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }

                    // 使用 Row 将两列按钮横向组合在中心
                    Row {
                        anchors.centerIn: parent
                        spacing: 20 // 两列之间的间距

                        // 第一列按钮
                        Column {
                            spacing: 16
                            Repeater {
                                model: ["前倾", "后仰", "击球", "备用1"]
                                delegate: controlButton
                            }
                        }

                        // 第二列按钮
                        Column {
                            spacing: 16
                            Repeater {
                                model: ["A", "B", "C", "D"]
                                delegate: controlButton
                            }
                        }
                    }
                }
    }
}
