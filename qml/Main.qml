import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    id: root
    visible: true
    width: 1200
    height: 800
    property alias theme: theme.currentTheme
    // Use theme colors defined in Theme.qml

    // instantiate theme
    Theme { id: theme }

    color: theme.background
    title: qsTr("Urdu Poetry Studio — POC")

    // Animated header
    Rectangle {
        id: header
        height: 64
        anchors.left: parent.left
        anchors.right: parent.right
        color: theme.header
        z: 2

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12
            anchors.left: parent.left
            anchors.leftMargin: 16

            Image { source: "qrc:/icons/logo.png"; width: 40; height: 40; fillMode: Image.PreserveAspectFit; visible: false } // optional
            Text { text: "Urdu Poetry Studio"; color: theme.textPrimary; font.pixelSize: 20; font.bold: true }
            Rectangle { width: 1; height: 28; color: theme.panelBorder; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "POC — Nastaliq rendering & PDF export"; color: theme.textSecondary; font.pixelSize: 12 }
        }

        // Right-side animated action
        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 16
            spacing: 8

            Button {
                id: themeBtn
                text: theme.currentTheme === "dark" ? "🌙" : "☀️"
                onClicked: theme.currentTheme = theme.currentTheme === "dark" ? "light" : "dark"
                background: Rectangle { color: theme.panel; radius: 6 }
            }

            Button {
                id: exportBtn
                anchors.verticalCenter: parent.verticalCenter
                text: "Export PDF"
                background: Rectangle { color: theme.accent; radius: 6 }
                onClicked: {
                    Backend.exportCurrentViewToPdf("poetryCanvas", "exported_poem.pdf", 300)
                }
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
            color: theme.panel
            border.color: theme.panelBorder
            Column {
                anchors.fill: parent; anchors.margins: 12
                spacing: 8
                Text { text: "BOOK\nProject"; color: theme.textPrimary; font.pixelSize: 14; wrapMode: Text.WordWrap }
                Flickable {
                    id: templates
                    anchors.left: parent.left; anchors.right: parent.right
                    contentHeight: columnContent.height
                    Column { id: columnContent; width: parent.width
                        Repeater { model: 6
                            Rectangle {
                                width: parent.width - 12; height: 64; color: theme.panel; radius: 6; anchors.horizontalCenter: parent.horizontalCenter; border.color: theme.panelBorder
                                Text { anchors.centerIn: parent; text: "Template " + (index+1); color: theme.textSecondary }
                                MouseArea { anchors.fill: parent; hoverEnabled: true;
                                    onEntered: parent.color = themeHoverColor()
                                    onExited: parent.color = theme.panel
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
            color: theme.background
            anchors.margins: 12
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: undefined
            anchors.right: undefined
            width: parent.width - 260 - 360
            radius: 8
            border.color: theme.panelBorder

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
                    color: theme.pageBackground
                    radius: 6
                    border.color: theme.pageBorder
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
                            color: theme.textPrimary
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
                            fontFamily: "Noto Nastaliq Urdu"
                            fontSize: 20
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
            color: theme.panel
            border.color: theme.panelBorder
            Column {
                anchors.fill: parent; anchors.margins: 12; spacing: 8
                Text { text: "Properties"; color: theme.textPrimary; font.pixelSize: 14 }
                Row { spacing: 8; Text { text: "Font:"; color: theme.textSecondary } ComboBox { id: fontBox; model: ["Noto Nastaliq Urdu", "System Nastaliq", "Scheherazade"]; onCurrentTextChanged: { editor.fontFamily = currentText } } }
                Row { spacing: 8; Text { text: "Size:"; color: theme.textSecondary } Slider { id: sizeSlider; from: 14; to: 36; value: 20; onValueChanged: editor.fontSize = value } }
                Rectangle { height: 1; color: theme.panelBorder }
                Text { text: "Preview"; color: theme.textSecondary }
                Rectangle { color: theme.previewBg; radius: 6; height: 120; anchors.left: parent.left; anchors.right: parent.right
                    Text { text: "یہ شاعری کا پری ویو ہے۔"; color: theme.textPrimary; anchors.centerIn: parent; font.pixelSize: 18 }
                }
            }
        }
    }

    function themeHoverColor() {
        return theme.currentTheme === "dark" ? "#14222e" : "#e8eef6";
    }
}
