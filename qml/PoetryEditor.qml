import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property alias sampleFile: sampleFilePath
    property string sampleFilePath: ""
    property string fontFamily: "Noto Nastaliq Urdu"
    property int fontSize: 20

    signal contentChanged(string content)

    Rectangle {
        anchors.fill: parent
        color: "transparent"
    }

    ScrollView {
        anchors.fill: parent
        Flickable {
            id: flick
            contentWidth: parent.width
            contentHeight: textItem.height
            clip: true

            TextEdit {
                id: textItem
                width: parent.width
                wrapMode: TextEdit.NoWrap
                readOnly: false
                color: "#222"
                font.family: root.fontFamily
                font.pixelSize: root.fontSize
                // Right-to-left for Urdu
                horizontalAlignment: Text.AlignRight
                textFormat: TextEdit.PlainText
                // Preserve line breaks — we rely on user to include them
                // Do not auto-justify here for POC
                placeholderText: "Paste or type your Urdu poetry here. Line breaks are preserved."
                onTextChanged: root.contentChanged(text)
            }

            Component.onCompleted: {
                if (root.sampleFilePath !== "") {
                    // Load sample file (attempt via XMLHttpRequest)
                    var xhr = new XMLHttpRequest()
                    xhr.open("GET", root.sampleFilePath)
                    xhr.onreadystatechange = function() {
                        if (xhr.readyState === XMLHttpRequest.DONE && xhr.status === 200) {
                            textItem.text = xhr.responseText
                        }
                    }
                    xhr.send()
                } else {
                    textItem.text = "یہ پہلی مصرع\nیہ دوسری مصرع\nیہ تیسری مصرع"
                }
            }
        }
    }
}
