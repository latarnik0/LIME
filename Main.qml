import QtQuick

Window {
    width: 350
    height: 800
    visible: true
    title: "lime_app"

    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput

    Rectangle {
        anchors.fill: parent
        anchors.margins: 10
        color: Qt.rgba(0.05, 0.05, 0.05, 0.85)
        radius: 15
        Text {
            anchors.centerIn: parent
            text: "minion zabojca"
            horizontalAlignment: Text.AlignHCenter
            color: "#00FFCC"
            font.pixelSize: 22
            font.bold: true
        }
    }
}