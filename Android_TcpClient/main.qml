import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import App.Network 1.0

ApplicationWindow {
    visible: true; width: 360; height: 640
    title: "ESP32 TCP 客户端"

    property bool isConnected: false
    property real keyboardHeight: Qt.inputMethod.keyboardRectangle.height

    TcpClient {
        id: client
        onDeviceAvailableChanged: {
            searchBtn.enabled = !deviceAvailable
            connectBtn.enabled = deviceAvailable
            statusText.text = deviceAvailable ? "设备在线" : "未找到设备"
        }
        onConnected: {
            isConnected = true
            connectBtn.text = "断开连接"
            statusText.text = "已连接"
        }
        onDisconnected: {
            isConnected = false
            connectBtn.text = "连接设备"
            statusText.text = "已断开"
        }
        onMessageReceived: {
            outputArea.text += msg + "\n"
        }
    }

    Flickable {
        anchors.fill: parent
        contentHeight: layoutContent.height + keyboardHeight
        clip: true
        anchors.margins: 20

        ColumnLayout {
            id: layoutContent
            width: parent.width
            spacing: 10

            Button { id: searchBtn; text: "搜索设备"
                onClicked: { statusText.text="正在搜索..."; client.scan() }
            }
            Button { id: connectBtn; enabled: false; text: "连接设备"
                onClicked: {
                    if (!isConnected) client.connectToDevice()
                    else client.disconnectFromDevice()
                }
            }
            Text { id: statusText; text:"准备就绪"; font.pointSize:14;
                   horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
            TextArea { id: outputArea; Layout.fillWidth: true; height:150; readOnly:true;
                       wrapMode: TextEdit.Wrap;
                       onTextChanged: positionY = contentHeight }
            TextField { id: inputField; Layout.fillWidth: true; placeholderText:"输入要发送的消息" }
            Button { text:"发送"; enabled:isConnected
                onClicked: {
                    if(inputField.text!=="") {
                        client.sendMessage(inputField.text)
                        //outputArea.text += "我: "+inputField.text+"\n"
                        inputField.clear()
                    }
                }
            }
            Rectangle { height: keyboardHeight; width:1; color:"transparent" }
        }
    }

    Connections {
        target: Qt.inputMethod
        onKeyboardRectangleChanged: keyboardHeight = Qt.inputMethod.keyboardRectangle.height
    }
}


