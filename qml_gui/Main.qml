import QtQuick
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

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
                            readonly property real maxRadius: 78
                            readonly property real minRadius: coreCount > 1 ? 22 : 55
                            readonly property real radiusStep: coreCount > 1
                                                               ? (maxRadius - minRadius) / (coreCount - 1)
                                                               : 0
                            property var ringAngles: []

                            FrameAnimation {
                                running: orbitHost.coreCount > 0
                                onTriggered: {
                                    const count = orbitHost.coreCount
                                    const usages = orbitHost.coreUsages
                                    const dt = frameTime
                                    const minDegPerSec = 25
                                    const maxDegPerSec = 270
                                    let angles = orbitHost.ringAngles.length === count
                                                 ? orbitHost.ringAngles.slice()
                                                 : Array(count).fill(0)
                                    for (let i = 0; i < count; ++i) {
                                        const raw = (usages && i < usages.length)
                                                    ? Number(usages[i])
                                                    : 0
                                        const usage = Number.isFinite(raw)
                                                      ? Math.max(0, Math.min(100, raw))
                                                      : 0
                                        const t = Math.pow(usage / 100, 1.2)
                                        const speed = minDegPerSec
                                                      + (maxDegPerSec - minDegPerSec) * t
                                        angles[i] = (angles[i] + speed * dt) % 360
                                    }
                                    orbitHost.ringAngles = angles
                                }
                            }

                            View3D {
                                anchors.fill: parent
                                camera: orbitCamera

                                environment: SceneEnvironment {
                                    clearColor: "transparent"
                                    backgroundMode: SceneEnvironment.Transparent
                                    antialiasingMode: SceneEnvironment.MSAA
                                    antialiasingQuality: SceneEnvironment.High
                                }

                                PerspectiveCamera {
                                    id: orbitCamera
                                    position: Qt.vector3d(0, 160, 125)
                                    eulerRotation.x: -50
                                }

                                DirectionalLight {
                                    eulerRotation.x: 0
                                    brightness: 1.4
                                    ambientColor: "#334455"
                                }

                                Repeater3D {
                                    model: orbitHost.coreCount

                                    Node {
                                        eulerRotation.x: orbitHost.coreCount > 0
                                                         ? index * 180 / orbitHost.coreCount
                                                         : 0
                                        eulerRotation.y: orbitHost.coreCount > 0
                                                         ? index * 360 / orbitHost.coreCount
                                                         : 0

                                        Model {
                                            eulerRotation.z: index < orbitHost.ringAngles.length
                                                             ? orbitHost.ringAngles[index]
                                                             : 0

                                            geometry: TorusGeometry {
                                                radius: orbitHost.minRadius
                                                        + index * orbitHost.radiusStep
                                                tubeRadius: 1.1
                                                rings: 72
                                                segments: 16
                                            }

                                            materials: PrincipledMaterial {
                                                baseColor: "#7FA8B8"
                                                roughness: 0.25
                                                metalness: 0.05
                                                lighting: PrincipledMaterial.FragmentLighting
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        visible: index === 3
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        IoChart {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            title: "Disk I/O"
                            upName: "rd"
                            downName: "wr"
                            upValue: root.lime ? root.lime.diskReadBps : 0
                            downValue: root.lime ? root.lime.diskWriteBps : 0
                            upColor: "#5EC8D8"
                            downColor: "#7FA8B8"
                            statsSource: root.lime
                        }

                        IoChart {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            title: "Net I/O"
                            upName: "rx"
                            downName: "tx"
                            upValue: root.lime ? root.lime.netRxBps : 0
                            downValue: root.lime ? root.lime.netTxBps : 0
                            upColor: "#5EC8D8"
                            downColor: "#7FA8B8"
                            statsSource: root.lime
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
