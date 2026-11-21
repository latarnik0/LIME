import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property string title: ""
    property string upName: ""
    property string downName: ""
    property real upValue: 0
    property real downValue: 0
    property color upColor: "#5EC8D8"
    property color downColor: "#7FA8B8"
    property int maxSamples: 90
    property var statsSource: null

    property var upHistory: []
    property var downHistory: []

    function formatRate(bps) {
        var v = Math.abs(bps)
        if (v < 1024)
            return v.toFixed(0) + " B/s"
        if (v < 1048576)
            return (v / 1024).toFixed(1) + " KB/s"
        return (v / 1048576).toFixed(2) + " MB/s"
    }

    function pushSample(up, down) {
        var u = upHistory.slice()
        var d = downHistory.slice()
        u.push(Math.max(0, up))
        d.push(Math.max(0, down))
        while (u.length > maxSamples)
            u.shift()
        while (d.length > maxSamples)
            d.shift()
        upHistory = u
        downHistory = d
        canvas.requestPaint()
    }

    Connections {
        target: root.statsSource
        function onStatsChanged() {
            root.pushSample(root.upValue, root.downValue)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: root.title
                color: "#EAFCff"
                font.pixelSize: 13
                Layout.fillWidth: true
            }
            Text {
                text: root.upName + " " + root.formatRate(root.upValue)
                color: root.upColor
                font.pixelSize: 11
            }
            Text {
                text: root.downName + " " + root.formatRate(root.downValue)
                color: root.downColor
                font.pixelSize: 11
            }
        }

        Canvas {
            id: canvas
            Layout.fillWidth: true
            Layout.fillHeight: true

            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()

            onPaint: {
                var ctx = getContext("2d")
                var w = width
                var h = height
                ctx.reset()
                ctx.clearRect(0, 0, w, h)

                var midY = h * 0.5
                ctx.strokeStyle = "#33FFFFFF"
                ctx.lineWidth = 1
                ctx.beginPath()
                ctx.moveTo(0, midY)
                ctx.lineTo(w, midY)
                ctx.stroke()

                var u = root.upHistory
                var d = root.downHistory
                var n = Math.max(u.length, d.length)
                if (n < 2)
                    return

                var peak = 1
                var i
                for (i = 0; i < u.length; ++i)
                    if (u[i] > peak)
                        peak = u[i]
                for (i = 0; i < d.length; ++i)
                    if (d[i] > peak)
                        peak = d[i]

                var amp = midY - 2
                var slots = Math.max(root.maxSamples - 1, 1)

                function xAt(idx, len) {
                    return ((root.maxSamples - len) + idx) / slots * w
                }

                function drawSeries(hist, upward, color) {
                    if (hist.length < 2)
                        return
                    ctx.strokeStyle = color
                    ctx.lineWidth = 1.5
                    ctx.beginPath()
                    for (var j = 0; j < hist.length; ++j) {
                        var x = xAt(j, hist.length)
                        var y = upward
                                 ? midY - (hist[j] / peak) * amp
                                 : midY + (hist[j] / peak) * amp
                        if (j === 0)
                            ctx.moveTo(x, y)
                        else
                            ctx.lineTo(x, y)
                    }
                    ctx.stroke()
                }

                drawSeries(u, true, root.upColor)
                drawSeries(d, false, root.downColor)
            }
        }
    }
}
