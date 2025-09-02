import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl
import MyApp 1.0    // BluetoothManager
import "."         // Joystick.qml

ApplicationWindow {
    id: appWin
    width: 800; height: 480
    visible: true
    title: qsTr("遥控器")
    flags: Qt.Window | Qt.CustomizeWindowHint

    property bool isConnected: false
    property string targetAddress: "00:15:83:40:03:29"
    property string logText: ""
    property string targetUuid: "00001101-0000-1000-8000-00805F9B34FB"  // 标准串口UUID


    function appendLog(msg) {
        const time = new Date().toLocaleTimeString()
        appWin.logText += `[${time}] ${msg}\n`

        // 自动滚动到底部
        Qt.callLater(() => {
            logArea.cursorPosition = logArea.length
        })
    }

    BluetoothManager {
        id: bt
        targetAddress: appWin.targetAddress

        onDeviceDiscovered: (name, addr) => {
            if (addr === targetAddress) {
                appendLog("发现目标设备，开始配对")
                bt.registerPairingAgent("8888")
            }
        }

        onPairingPinRequested: (pin) => {
            appendLog("配对请求 PIN 码: " + pin)
            pairingDialog.pin = pin
            pairingDialog.open()
        }

        onPaired: {
            appendLog("配对成功，搜索服务…")
            appWin.pairedLogged = true
        }

        onConnected: {
            appendLog("已连接到设备")
            isConnected = true
        }

        onDisconnected: {
            appendLog("已断开连接")
            isConnected = false
            appWin.pairedLogged = false  // 重置状态
            bt.startScan()
        }

        onErrorOccurred: function(err) {
            var errorMsg = String(err)
            appendLog("蓝牙错误: " + errorMsg)
        }
    }

    Dialog {
        id: pairingDialog
        title: "蓝牙配对"
        standardButtons: Dialog.Ok
        property string pin: ""

        Column {
            spacing: 10
            Text { text: "请输入以下 PIN 码完成配对：" }
            Text {
                text: pairingDialog.pin
                font.pixelSize: 24
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    property real left_stickAngle: 0
    property real left_stickDistance: 0
    property real right_stickAngle: 0
    property real right_stickDistance: 0

    property var buttonStates: {
        "前倾": false, "后仰": false,
        "击球": false, "备用1": false
    }

    property int sum: 0
    property int num: 0

    Component.onCompleted: {
        bt.startScan()
    }

    Timer {
        interval: 10; running: true; repeat: true
        onTriggered: {
            if (isConnected) {
                // var a = buttonStates["前倾"] ? 1 : 0
                // var b = buttonStates["后仰"] ? 1 : 0
                // var c = buttonStates["击球"] ? 1 : 0
                // var d = buttonStates["备用1"] ? 1 : 0

                // const dataPart = `[${left_stickAngle.toFixed(1)};${left_stickDistance.toFixed(2)};${a};${b};${c};${d}]`
                const dataPart = `[${left_stickAngle.toFixed(1)};${left_stickDistance.toFixed(2)};${right_stickAngle.toFixed(1)};${right_stickDistance.toFixed(2)};${0};${0}]`
                const crc = calculateCRC(dataPart)
                const payload = `${dataPart}${crc}\n`

                bt.sendMessage(payload)
                sum++
            }
        }
    }

    function calculateCRC(str) {
        let crc = 0xFFFF
        for (let i = 0; i < str.length; i++) {
            crc ^= str.charCodeAt(i)
            for (let j = 0; j < 8; j++) {
                const bit = crc & 1
                crc >>= 1
                if (bit) crc ^= 0xA001
            }
        }
        const lo = crc & 0xFF
        const hi = (crc >> 8) & 0xFF
        return hi.toString(16).padStart(2,'0').toUpperCase()
             + lo.toString(16).padStart(2,'0').toUpperCase()
    }

    Row {
        anchors.fill: parent; anchors.margins: 40
        spacing: parent.width * 0.05

        Item {
            width: parent.width * 0.25
            height: width
            anchors.verticalCenter: parent.verticalCenter
            Joystick {
                id: leftStick
                anchors.fill: parent
                onChanged: {
                    left_stickAngle = angle
                    left_stickDistance = distance
                }
            }
        }

        Item {
            width: parent.width * 0.3
            height: parent.width * 0.4
            anchors.verticalCenter: parent.verticalCenter

            Column {
                anchors.centerIn: parent
                spacing: 12
                width: parent.width

                Text {
                    text: "角度: " + left_stickAngle.toFixed(1) + "°/" + right_stickAngle.toFixed(1) + "°"
                    font.pixelSize: 24
                }

                Text {
                    text: "距离: " + left_stickDistance.toFixed(3) + "/" + right_stickDistance.toFixed(3)
                    font.pixelSize: 24
                }

                Rectangle {
                    width: parent.width
                    height: 120
                    color: "#f0f0f0"
                    radius: 6

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 4

                        TextArea {
                            id: logArea
                            text: appWin.logText
                            readOnly: true
                            wrapMode: TextArea.Wrap
                            font.pixelSize: 14
                            background: null
                        }
                    }
                }

                Button {
                    width: 160
                    height: 48
                    text: isConnected ? qsTr("断开连接") : qsTr("连接设备")
                    onClicked: {
                        if (isConnected) {
                            appendLog("手动断开连接")
                            bt.disconnect()
                        } else if (targetAddress !== "") {
                            appendLog("开始连接: " + targetAddress)
                            bt.connectToDevice(targetAddress, targetUuid) // 修改此处，传入 UUID
                        } else {
                            appendLog("未设置目标地址")
                        }
                    }
                }
            }
        }

        // Item {
        //     width: parent.width * 0.25
        //     height: parent.width * 0.25
        //     anchors.verticalCenter: parent.verticalCenter

        //     Column {
        //         anchors.centerIn: parent; spacing: 16
        //         Repeater {
        //             model: ["前倾", "后仰", "击球", "备用1"]
        //             Button {
        //                 text: modelData; checkable: true
        //                 width: 80; height: 48
        //                 onPressed:  buttonStates[modelData] = true
        //                 onReleased: buttonStates[modelData] = false
        //                 background: Rectangle {
        //                     color: pressed ? "#4CAF50" : "#E0E0E0"
        //                     radius: 4
        //                 }
        //                 contentItem: Text {
        //                     text: parent.text
        //                     color: parent.pressed ? "white" : "black"
        //                     horizontalAlignment: Text.AlignHCenter
        //                     verticalAlignment: Text.AlignVCenter
        //                 }
        //             }
        //         }
        //     }
        // }

        Item {
            width: parent.width * 0.25
            height: width
            anchors.verticalCenter: parent.verticalCenter
            Joystick {
                id: rightStick
                anchors.fill: parent
                onChanged: {
                    right_stickAngle = angle
                    right_stickDistance = distance
                }
            }
        }

    }
}
