// Joystick.qml
import QtQuick 2.15

Item {
    id: root
    // 组件尺寸应由外部容器控制
    width: parent ? parent.width : 200
    height: parent ? parent.height : 200

    /// 输出属性
    property real angle: 0        // -180…+180°, 0° 向上，顺时针为正
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
        // 计算偏移
        var dx = pos.x - _center
        var dy = pos.y - _center

        // 距离及限制
        var r = Math.sqrt(dx*dx + dy*dy)
        var limited = Math.min(r, _maxRadius)
        distance = limited / _maxRadius

        // 计算角度：0° 指向正上，顺时针为正，范围已经在 [-180,+180]
        var rawAngle = Math.atan2(dx, -dy) * 180 / Math.PI
        angle = rawAngle

        // 根据角度和距离设置 knob 位置
        var rad = angle * Math.PI / 180
        knob.x = _center + limited * Math.sin(rad) - knob.width/2
        knob.y = _center - limited * Math.cos(rad) - knob.height/2

        root.changed()
    }
}

