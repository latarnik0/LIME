import QtQuick
import QtQuick.Layouts

Window {
    id: root

    readonly property var lime: typeof Lime !== "undefined" ? Lime : null

    width: 1000
    height: 1000
    minimumWidth: 480
    minimumHeight: 320
    visible: true
    title: "lime"
    color: Qt.rgba(0, 0, 0, 0.95)
    flags: Qt.Window | Qt.FramelessWindowHint

    MouseArea {
        anchors.fill: parent
        z: 0
        property real px: 0
        property real py: 0
        onPressed: (m) => { px = m.x; py = m.y }
        onPositionChanged: (m) => {
            if (!pressed) return
            root.x += m.x - px
            root.y += m.y - py
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10
        z: 1

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            Layout.maximumHeight: 36
            color: "transparent"
            border.color: "#33FFFFFF"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 16

                Text {
                    text: root.lime ? root.lime.hostname : "—"
                    color: "#EAFCff"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    Layout.preferredWidth: 120
                }
                Text {
                    text: "up " + (root.lime ? root.lime.uptime : "—")
                    color: "#7FA8B8"
                    font.pixelSize: 12
                }
                Text {
                    text: root.lime ? root.lime.battery : "—"
                    color: "#7FA8B8"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Text {
                    text: root.lime ? root.lime.kernelVersion : "—"
                    color: "#7FA8B8"
                    font.pixelSize: 12
                    elide: Text.ElideLeft
                    horizontalAlignment: Text.AlignRight
                    Layout.preferredWidth: 180
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 2
            rows: 2
            columnSpacing: 10
            rowSpacing: 10

            Repeater {
                model: 4
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "transparent"
                    border.color: "#33FFFFFF"
                    border.width: 1
                    radius: 0

                    ColumnLayout {
                        visible: index === 0
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: "CPU " + (root.lime ? root.lime.cpuUsage.toFixed(0) : "—") + "%"
                                color: "#EAFCff"
                                font.pixelSize: 14
                                Layout.fillWidth: true
                            }
                            Text {
                                text: root.lime
                                      ? (root.lime.cpuTemp + "°C  ·  "
                                         + (root.lime.cpuFreqMhz >= 1000
                                            ? (root.lime.cpuFreqMhz / 1000).toFixed(2) + " GHz"
                                            : root.lime.cpuFreqMhz + " MHz"))
                                      : "—"
                                color: "#7FA8B8"
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignRight
                            }
                        }

                        Item {
                            id: orbitHost
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            readonly property var coreUsages: root.lime ? root.lime.perCoreUsage : []
                            readonly property int coreCount: coreUsages.length
                            readonly property real maxRadius: Math.max(0, Math.min(width, height) * 0.38 - 4)
                            readonly property real minRadius: coreCount > 1 ? maxRadius * 0.28 : maxRadius * 0.7
                            readonly property real radiusStep: coreCount > 1
                                                               ? (maxRadius - minRadius) / (coreCount - 1)
                                                               : 0

                            Repeater {
                                model: orbitHost.coreCount

                                Item {
                                    id: ring
                                    anchors.centerIn: parent
                                    width: orbitHost.width
                                    height: orbitHost.height

                                    readonly property real orbitRadius: orbitHost.minRadius
                                                                        + index * orbitHost.radiusStep
                                    // Static phase only (step 1); rotation drive comes later.
                                    rotation: index * 18

                                    Canvas {
                                        id: ringCanvas
                                        anchors.fill: parent
                                        antialiasing: true

                                        onPaint: {
                                            const ctx = getContext("2d")
                                            ctx.reset()
                                            if (ring.orbitRadius <= 0)
                                                return

                                            const cx = width * 0.5
                                            const cy = height * 0.5
                                            const r = ring.orbitRadius

                                            ctx.strokeStyle = "#7FA8B8"
                                            ctx.lineWidth = 1.25
                                            ctx.beginPath()
                                            ctx.arc(cx, cy, r, 0, Math.PI * 2, false)
                                            ctx.stroke()
                                        }

                                        Component.onCompleted: requestPaint()
                                    }

                                    onOrbitRadiusChanged: ringCanvas.requestPaint()
                                    onWidthChanged: ringCanvas.requestPaint()
                                    onHeightChanged: ringCanvas.requestPaint()
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    MouseArea {
        width: 16
        height: 16
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 2
        cursorShape: Qt.SizeFDiagCursor
        property real sx: 0
        property real sy: 0
        property int sw: 0
        property int sh: 0
        onPressed: (m) => {
            sx = m.x; sy = m.y
            sw = root.width; sh = root.height
        }
        onPositionChanged: (m) => {
            if (!pressed) return
            root.width = Math.max(root.minimumWidth, sw + (m.x - sx))
            root.height = Math.max(root.minimumHeight, sh + (m.y - sy))
        }
    }
}
