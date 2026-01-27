import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl
import QtQuick.Layouts 1.15
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

    property bool isBusy: false

    property bool pairedLogged: false

    property var inputStates:{
        "上升": false, "下降": false,
        "左旋": false, "右旋": false
    }

    property bool tx_monitor_up: false
    property bool tx_monitor_down: false
    property bool tx_monitor_left: false
    property bool tx_monitor_right: false

    property real left_stickAngle: 0
    property real left_stickDistance: 0
    property real right_stickAngle: 0
    property real right_stickDistance: 0

    property int sum: 0
    property int num: 0

    function appendLog(msg) {
        const time = new Date().toLocaleTimeString()
        appWin.logText += `[${time}] ${msg}\n`

        // 自动滚动到底部
        Qt.callLater(() => {
            logArea.cursorPosition = logArea.length
        })
    }

    function forceResetState()
    {
        // 1.停止看门狗
        disconnectWatchdog.stop()

        // 2.释放 UI 锁
        isBusy = false
        isConnected = false

        if(appWin.pairedLogged !== undefined) {
                     appWin.pairedLogged = false
        }

        // 3.状态处理
        tx_monitor_up = false
        tx_monitor_down = false
        tx_monitor_left = false
        tx_monitor_right = false

        // 4.重新搜索
        bt.startScan()
    }

    // 看门狗
    Timer{
        id: disconnectWatchdog
        interval: 1500
        repeat: false
        onTriggered: {
            appendLog("断开超时，准备强制断开")
            bt.abortconnection()
            forceResetState()
        }
    }

    Component.onCompleted: {
        bt.startScan()
    }

    BluetoothManager {
        id: bt
        targetAddress: appWin.targetAddress

        onDeviceDiscovered: (name, addr) => {
            if (addr === targetAddress) {
                appendLog("发现目标设备，开始配对")
                bt.registerPairingAgent()
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
            isBusy = false    // 解锁UI
            disconnectWatchdog.stop()  // 停止看门狗
        }

        onDisconnected: {
            appendLog("已断开连接")
            isConnected = false
            forceResetState()
        }

        onErrorOccurred: function(err) {
            appendLog("蓝牙错误: " + err)
            forceResetState()
        }
    }

    Timer {
        interval: 20; running: true; repeat: true
        onTriggered: {
            if (!isConnected) {
                    // 你的代码第 18-19 行已经做了部分清除，但不够彻底
                    // 建议在这里显式复位所有控制位
                    tx_monitor_up = false
                    tx_monitor_down = false
                    tx_monitor_left = false
                    tx_monitor_right = false
                    return // 直接返回，不执行发送
            }
            if (isConnected) {
                let b_up = inputStates["上升"] ? 1 : 0
                let b_down = inputStates["下降"] ? 1 : 0
                let b_left = inputStates["左旋"] ? 1 : 0
                let b_right = inputStates["右旋"] ? 1 : 0

                tx_monitor_up = b_up
                tx_monitor_down = b_down
                tx_monitor_left = b_left
                tx_monitor_right = b_right

                bt.sendControlData(left_stickAngle, left_stickDistance,
                                   b_up, b_down, b_left, b_right)
                sum++
            }
            else {
                if(tx_monitor_up) tx_monitor_up = false
                if(tx_monitor_down) tx_monitor_down = false
                if(tx_monitor_left) tx_monitor_left = false
                if(tx_monitor_right) tx_monitor_right = false
            }
        }
    }

    Dialog {
        id: pairingDialog
        title: "蓝牙配对"
        standardButtons: Dialog.Ok
        property string pin: ""

        Column {
            anchors.centerIn: parent
            spacing: 10
            Text {
                text: "请输入以下 PIN 码完成配对："
                Layout.alignment: Qt.AlignHCenter
            }
            Text {
                text: pairingDialog.pin
                font.pixelSize: 24
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                // anchors.horizontalCenter: parent.horizontalCenter
            }
        }
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

                // Text {
                //     text: "角度: " + left_stickAngle.toFixed(1) + "°/" + right_stickAngle.toFixed(1) + "°"
                //     font.pixelSize: 24
                // }

                // Text {
                //     text: "距离: " + left_stickDistance.toFixed(3) + "/" + right_stickDistance.toFixed(3)
                //     font.pixelSize: 24
                // }

                Text {
                    text: "角度: " + left_stickAngle.toFixed(1) + "°"
                    font.pixelSize: 24
                }

                Text {
                    text: "距离: " + left_stickDistance.toFixed(3)
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

        Item {
            width: parent.width * 0.25
            height: parent.width * 0.25
            anchors.verticalCenter: parent.verticalCenter

            Column {
                anchors.centerIn: parent; spacing: 16
                Repeater {
                    model: ["上升", "下降", "左旋", "右旋"]
                    Button {
                        text: modelData;
                        width: 80; height: 48
                        onPressedChanged: {
                                inputStates[modelData] = pressed
                        }
                        property bool isActiveData: {
                            switch(modelData)
                            {
                                case "上升": return tx_monitor_up;
                                case "下降": return tx_monitor_down;
                                case "左旋": return tx_monitor_left;
                                case "右旋": return tx_monitor_right;
                                default: return false;
                            }
                        }

                        background: Rectangle {
                            color: parent.isActiveData ? "#4CAF50" : "#E0E0E0"
                            radius: 4

                            border.width: parent.pressed ? 2 : 0
                            border.color: "#2E7D32"
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

        // Item {
        //     width: parent.width * 0.25
        //     height: width
        //     anchors.verticalCenter: parent.verticalCenter
        //     Joystick {
        //         id: rightStick
        //         anchors.fill: parent
        //         onChanged: {
        //             right_stickAngle = angle
        //             right_stickDistance = distance
        //         }
        //     }
        // }

    }
}
