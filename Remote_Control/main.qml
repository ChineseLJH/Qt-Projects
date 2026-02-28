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
    property int sum: 0
    property int num: 0

    // 【核心修改 1】实例化新的底层 UDP 数据发送类
    UdpDataClient { id: udpData }

    // —— 接收信号 —— //
    Connections {
        target: udpData  // 【核心修改 2】绑定目标更改为 udpData
        onDataReceived: function(data) {
            // 获取当前时间
            const now = new Date()
            const timeString = now.getHours().toString().padStart(2, '0') + ":" +
                               now.getMinutes().toString().padStart(2, '0') + ":" +
                               now.getSeconds().toString().padStart(2, '0') + "." +
                               now.getMilliseconds().toString().padStart(3, '0')

            console.log("接收到数据:", data, "时间:", timeString)

            if (parseInt(data) > num)
                num = parseInt(data)
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


    // —— 50Hz 发送定时器 —— //
    Timer {
        interval: 20;
        running: true; repeat: true
        onTriggered: {
            if (udpData.connected) {  // 【核心修改 3】引用的状态标志替换
                const a = buttonStates["前倾"] ? 1 : 0
                const b = buttonStates["后仰"] ? 1 : 0
                const c = buttonStates["击球"] ? 1 : 0
                const d = buttonStates["备用1"] ? 1 : 0

                // 构造数据部分（不包含换行符）
                const dataPart = `[${stickAngle.toFixed(1)};${stickDistance.toFixed(2)};${a};${b};${c};${d}]`

                // 计算CRC并拼接
                const crc = calculateCRC(dataPart)
                const payload = `${dataPart}${crc}\n`

                udpData.sendMessage(payload) // 【核心修改 4】执行无状态的 UDP 数据报发送
                sum++
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
                    text: (sum === 0)
                        ? "丢包: N/A"
                        : "丢包: " + ((1 - num / sum) * 100).toFixed(2) + "%"
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
        Item {
            id: buttonPanel
            width: parent.width * 0.25
            anchors.verticalCenter: parent.verticalCenter
            height: joystickContainer.height

            Column {
                anchors.centerIn: parent
                spacing: 16

                Repeater {
                    model: ["前倾", "后仰", "击球", "备用1"]
                    Button {
                        text: modelData
                        width: 80; height: 48

                        // 监听底层 pressed 属性变更与系统级取消事件
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
            }
        }
    }
}
