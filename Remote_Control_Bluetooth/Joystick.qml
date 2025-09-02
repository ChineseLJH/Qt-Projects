// Joystick.qml
import QtQuick 2.15

Item {
    id: root
    // 组件尺寸应由外部容器控制
    width: parent ? parent.width : 200
    height: parent ? parent.height : 200

    /// 输出属性
    property real angle: 0        // 360°，逆时针为正
    property real distance: 0     // 0–1，单位圆上的比例
    signal changed()

    // 内部计算用
    property real _knobSize: Math.min(width, height) * 0.2    // 摇杆小球直径占 20%
    property real _center: width / 2
    property real _maxRadius: _center - _knobSize / 2
    property bool _active: false

    // 底盘圆形
    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "#333"
    }

    // 摇杆小球
    Rectangle {
        id: knob
        width: _knobSize
        height: _knobSize
        radius: _knobSize / 2
        color: "#AAA"
        x: _center - width / 2
        y: _center - height / 2
        z: 1
    }

    // 触摸区域
    MultiPointTouchArea {
        anchors.fill: parent
        minimumTouchPoints: 1
        maximumTouchPoints: 1

        onPressed: {
            if (touchPoints.length > 0) {
                _active = true
                updateStick(Qt.point(touchPoints[0].x, touchPoints[0].y))
            }
        }
        onTouchUpdated: {
            if (_active && touchPoints.length > 0) {
                updateStick(Qt.point(touchPoints[0].x, touchPoints[0].y))
            }
        }
        onReleased: {
            _active = false
            // 复位摇杆
            knob.x = _center - knob.width / 2
            knob.y = _center - knob.height / 2
            angle = 0
            distance = 0
            root.changed()
        }
    }

    function updateStick(pos) {
        var dx = pos.x - _center
        var dy = pos.y - _center

        // 距离与限制
        var r = Math.sqrt(dx*dx + dy*dy)
        var limited = Math.min(r, _maxRadius)
        distance = limited / _maxRadius

        // 角度：0° = 正上方，逆时针递增到 360°
        // 先取标准极角 beta（+X 为 0°，逆时针为正）
        var beta = Math.atan2(dy, dx) * 180 / Math.PI
        // 把基准从 +X 旋到 -Y（正上方），并保持逆时针为正
        angle = (-beta - 90 + 360) % 360

        // 小球位置：直接沿触点方向等比例缩放到圆内（不会镜像）
        var scale = r > 0 ? (limited / r) : 0
        knob.x = _center + dx * scale - knob.width / 2
        knob.y = _center + dy * scale - knob.height / 2

        root.changed()
    }
}
