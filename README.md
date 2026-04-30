# Qt-Projects

这是一个汇总仓库，包含作者在大学期间用 Qt（C++/QML）完成的一些练手项目与课程作业。

仓库结构（项目列表与简要说明）：

- Android_TcpClient — Android 平台的 TCP 客户端（Qt + QML）。用于与 ESP32 等设备建立 TCP 连接、发现设备并发送/接收消息。主要文件：`Android_TcpClient/main.qml`（UI）和 `Android_TcpClient/TcpClient.cpp`（连接与收发逻辑）。
- KeShe — 课设演示的桌面应用（手办自助售卖系统）。包含商品网格展示、搜索、库存管理与支付扫码对接示例。主要文件：`KeShe/mainwindow.cpp`、`KeShe/inventorymanager.*`、`KeShe/figuredetaildialog.*`。
- Remote_Control — 基于 UDP 的遥控器示例（QML 界面）。实现了广播发现、摇杆输入、CRC 校验与 50Hz 控制包发送。主要文件：`Remote_Control/main.qml`（控制界面）、`Remote_Control/myudp.cpp`（UDP 发现）。
- Remote_Control_Bluetooth — 使用蓝牙 RFCOMM 的遥控/通信示例。实现设备扫描、配对、连接、数据帧（含 CRC16）发送。主要文件：`Remote_Control_Bluetooth/bluetoothmanager.cpp`。
- Serial_Assistant — 桌面串口助手（调试工具）。功能包括检测串口、打开/关闭、发送/接收、16 进制显示与发送、主题切换等。主要文件：`Serial_Assistant/mywidget.cpp`。
- TCP_Client — 桌面 TCP 客户端示例（Qt Widgets），用于连接服务器并发送/接收文本数据。入口：`TCP_Client/main.cpp`、界面文件 `TCP_Client/mywidget.*`。
- TCP_Server — 桌面 TCP 服务端示例，支持监听、管理多客户端连接并向指定客户端或全部广播消息。主要文件：`TCP_Server/mywidget.cpp`。
- UDP — 通用的 UDP 通信示例，包含发送/接收演示界面。入口：`UDP/main.cpp`、`UDP/mywidget.*`。
- Web_Assistants — 一些与网络/网页交互相关的示例应用，包含基础的网络请求与展示逻辑。主要文件位于 `Web_Assistants/` 目录。

构建与运行

- 所有项目均基于 Qt（6.7.3）开发。使用 Qt Creator 打开对应的 `.pro` 项目文件进行构建与运行。
- 通常步骤：
  1. 在 Qt Creator 中选择合适的 Kit（例如 Desktop 或 Android）。
  2. 点击 Build / Run 或先构建再运行。
  3. 对于命令行构建，可在项目目录下使用 `qmake`/`cmake` + `make`，或使用 Qt Creator 自动生成的构建配置。

