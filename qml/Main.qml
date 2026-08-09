import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    id: root
    visible: true
    width: 1200
    height: 800
    color: "#111215"
    title: qsTr("Urdu Poetry Studio — POC")

    // Animated header
    Rectangle {
        id: header
        height: 64
        anchors.left: parent.left
        anchors.right: parent.right
        color: "#0f1720"
        z: 2

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12
            anchors.left: parent.left
            anchors.leftMargin: 16

            Image { source: "qrc:/icons/logo.png"; width: 40; height: 40; fillMode: Image.PreserveAspectFit; visible: false } // optional
            Text { text: "Urdu Poetry Studio"; color: "#e6eef6"; font.pixelSize: 20; font.bold: true }
            Rectangle { width: 1; height: 28; color: "#263042"; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "POC — Nastaliq rendering & PDF export"; color: "#9fb0c9"; font.pixelSize: 12 }
        }

        // Right-side animated action
        Button {
            id: exportBtn
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 16
            text: "Export PDF"
            background: Rectangle { color: "#1f6feb"; radius: 6 }
            onClicked: {
                // Call C++ backend; output file in app dir as exported.pdf
                Backend.exportCurrentViewToPdf("poetryCanvas", "exported_poem.pdf", 300)
            }
        }
    }

    // Main split layout
    Row {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        // Left panel: project tree placeholder
        Rectangle {
            width: 260
            color: "#0b0f14"
            border.color: "#1b2430"
            Column {
                anchors.fill: parent; anchors.margins: 12
                spacing: 8
                Text { text: "BOOK\nProject"; color: "#bcd0e6"; font.pixelSize: 14; wrapMode: Text.WordWrap }
                // Animated template thumbnails area (placeholder)
                Flickable {
                    id: templates
                    anchors.left: parent.left; anchors.right: parent.right
                    contentHeight: columnContent.height
                    Column { id: columnContent; width: parent.width
                        Repeater { model: 6
                            Rectangle {
                                width: parent.width - 12; height: 64; color: "#0d1720"; radius: 6; anchors.horizontalCenter: parent.horizontalCenter
                                Text { anchors.centerIn: parent; text: "Template " + (index+1); color: "#cfe3f8" }
                                MouseArea { anchors.fill: parent; hoverEnabled: true;
                                    onEntered: parent.color = "#14222e"
                                    onExited: parent.color = "#0d1720"
                                    onClicked: console.log("Apply template", index+1)
                                }
                            }
                            Rectangle { height: 8; color: "transparent" }
                        }
                    }
                }
            }
        }

        // Center canvas
        Rectangle {
            id: canvasArea
            color: "#0f1720"
            anchors.margins: 12
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: undefined
            anchors.right: undefined
            width: parent.width - 260 - 360
            radius: 8
            border.color: "#233041"

            // Canvas with animated entrance
            Item {
                id: contentRoot
                anchors.fill: parent
                anchors.margins: 24

                // Page background
                Rectangle {
                    id: page
                    width: Math.min(parent.width - 48, 640)
                    height: Math.min(parent.height - 48, 880)
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#fffaf6"
                    radius: 6
                    border.color: "#ddd2c8"
                    elevation: 4

                    // This is the QML item that backend will find and export
                    Item {
                        id: poetryCanvas
                        objectName: "poetryCanvas"
                        anchors.fill: parent
                        anchors.margins: 40

                        // Title
                        Text {
                            id: titleText
                            text: "آئینہ فروش"
                            font.family: "Noto Nastaliq Urdu"
                            font.pixelSize: 34
                            color: "#2b2b2b"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        // Poetry editor area
                        PoetryEditor {
                            id: editor
                            anchors.top: titleText.bottom
                            anchors.topMargin: 12
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            sampleFile: "sample_poem.txt"
                        }
                    }
                }
            }

            // subtle float animation
            SequentialAnimation running: true looping: true {
                NumberAnimation target: contentRoot.opacity from: 0.98 to: 1.0 duration: 2400 easing.type: Easing.InOutQuad
            }
        }

        // Right properties/inspector
        Rectangle {
            width: 360
            color: "#091017"
            border.color: "#12202c"
            Column {
                anchors.fill: parent; anchors.margins: 12; spacing: 8
                Text { text: "Properties"; color: "#cfe3f8"; font.pixelSize: 14 }
                // Font selector (simple)
                Row { spacing: 8; Text { text: "Font:"; color: "#9fb0c9" } ComboBox { id: fontBox; model: ["Noto Nastaliq Urdu", "System Nastaliq", "Scheherazade"]; onCurrentTextChanged: { editor.fontFamily = currentText } } }
                Row { spacing: 8; Text { text: "Size:"; color: "#9fb0c9" } Slider { id: sizeSlider; from: 14; to: 36; value: 20; onValueChanged: editor.fontSize = value } }
                Rectangle { height: 1; color: "#122533" }
                Text { text: "Preview"; color: "#9fb0c9" }
                Rectangle { color: "#0b1014"; radius: 6; height: 120; anchors.left: parent.left; anchors.right: parent.right
                    Text { text: "یہ شاعری کا پری ویو ہے۔"; color: "#dbeafc"; anchors.centerIn: parent; font.pixelSize: 18 }
                }
            }
        }
    }
}
