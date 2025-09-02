import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import MyApp 1.0    // TcpClient
import "."         // Joystick.qml

ApplicationWindow {
    id: appWin
    width: 800; height: 480
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
        udp.startListening(54321)  // ✅ 监听 ESP32 的广播
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

    TcpClient { id: tcp }

    // —— 接收信号 —— //
    // Connections {
    //     target: tcp
    //     onDataReceived: function(data) {
    //         if(parseInt(data) > num)
    //             num = parseInt(data)  // 把ESP32发回来的“已收到总数”存为 num
    //     }
    // }

    Connections {
        target: tcp
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
            // return lo.toString(16).padStart(2, '0').toUpperCase() +
            //        hi.toString(16).padStart(2, '0').toUpperCase();
            return  hi.toString(16).padStart(2, '0').toUpperCase()+
                    lo.toString(16).padStart(2, '0').toUpperCase();
            // return crc;
        }


    // —— 50Hz 发送定时器 —— //
    Timer {
        interval: 20; running: true; repeat: true
        onTriggered: {
            if (tcp.connected) {
                const a = buttonStates["前倾"] ? 1 : 0
                const b = buttonStates["后仰"] ? 1 : 0
                const c = buttonStates["击球"] ? 1 : 0
                const d = buttonStates["备用1"] ? 1 : 0


                // const payload = [${stickAngle.toFixed(1)};${stickDistance.toFixed(2)};${a};${b};${c};${d}]\n
                // tcp.sendMessage(payload)
                // sum++

                // 构造数据部分（不包含换行符）
                const dataPart = `[${stickAngle.toFixed(1)};${stickDistance.toFixed(2)};${a};${b};${c};${d}]`

                // 计算CRC并拼接
                const crc = calculateCRC(dataPart)
                const payload = `${dataPart}${crc}\n`

                tcp.sendMessage(payload)
                sum++
            }
        }
    }

    // // —— 50Hz 发送定时器 —— //
    // Timer {
    //     interval: 20; running: true; repeat: true
    //     onTriggered: {
    //         if (tcp.connected) {
    //             const a = buttonStates.A ? 1 : 0
    //             const b = buttonStates.B ? 1 : 0
    //             const c = buttonStates.C ? 1 : 0
    //             const d = buttonStates.D ? 1 : 0

    //             const payload = [${stickAngle.toFixed(1)};${stickDistance.toFixed(2)};${a};${b};${c};${d}]\n
    //             tcp.sendMessage(payload)
    //             sum++
    //         }
    //     }
    // }

    Row {
        anchors.fill: parent
        anchors.margins: 40
        spacing: parent.width * 0.05  // 三部分之间自动调整间距

        // ✅ 左侧 Joystick 容器
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

        // ✅ 中间信息区容器（保持内容居中）
        Item {
            id: centerContainer
            width: parent.width * 0.3
            anchors.verticalCenter: parent.verticalCenter
            height: joystickContainer.height  // 保持高度一致，或你可以定固定高

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
                // Text {
                //     text: (sum === 0)
                //         ? "丢包：N/A；N=" + num + "，S=" + sum
                //         : "丢包：" + ((1 - num / sum) * 100).toFixed(2) + "%；N=" + num + "，S=" + sum
                //     font.pixelSize: 24
                // }


                Button {
                    width: 160; height: 48
                    text: tcp.connected ? qsTr("断开连接") : qsTr("连接设备")
                    onClicked: {
                        if (tcp.connected)
                            tcp.disconnectFromHost()
                        else if (targetIp !== "")
                            tcp.connectToHost(targetIp, 12345)
                        else
                            console.log("还未收到设备 IP，无法连接")
                    }
                }
            }
        }

        // ✅ 右侧按钮容器
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
                        checkable: true
                        width: 80; height: 48
                        onPressed: buttonStates[modelData] = true
                        onReleased: buttonStates[modelData] = false

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
