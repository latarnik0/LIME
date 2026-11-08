import QtQuick

Window {
    id: root

    width: 800
    height: 560
    visible: true
    title: "lime"
    color: Qt.rgba(0, 0, 0, 0.85)
    flags: Qt.Window | Qt.FramelessWindowHint

    MouseArea {
        anchors.fill: parent
        property real px: 0
        property real py: 0
        onPressed: (m) => { px = m.x; py = m.y }
        onPositionChanged: (m) => {
            if (!pressed) return
            root.x += m.x - px
            root.y += m.y - py
        }
    }
}
