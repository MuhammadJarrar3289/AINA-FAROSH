import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: fm
    width: parent.width
    height: parent.height
    color: "transparent"

    Column {
        anchors.fill: parent
        spacing: 8

        Text { text: "Font Manager"; color: theme.textPrimary; font.pixelSize: 16 }

        Row { spacing: 8
            Button {
                text: "Scan fonts in ./fonts/"
                onClicked: {
                    var res = Project.scanFonts();
                    fontsModel.clear();
                    for (var i=0;i<res.length;i++) {
                        fontsModel.append(res[i]);
                    }
                }
            }
            Button {
                text: "Load manifest"
                onClicked: {
                    Project.loadManifest("");
                    var res = Project.scanFonts();
                    fontsModel.clear();
                    for (var i=0;i<res.length;i++) {
                        fontsModel.append(res[i]);
                    }
                }
            }
            Button {
                text: "Save manifest"
                onClicked: {
                    Project.saveManifest("");
                }
            }
        }

        ListView {
            id: list
            model: fontsModel
            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
            delegate: Rectangle {
                width: parent.width; height: 64; color: theme.panel; border.color: theme.panelBorder; radius: 6
                Row { anchors.fill: parent; anchors.margins: 8; spacing: 8; 
                    Column { width: parent.width-120; Text { text: name; color: theme.textPrimary } Text { text: fileName + ' — ' + reason; color: theme.textSecondary; font.pixelSize: 11; elide: Text.ElideRight } }
                    Column { width: 100; spacing: 4; anchors.verticalCenter: parent.verticalCenter
                        Row { spacing: 6; Text { text: "Auto:"; color: theme.textSecondary } Text { text: auto_detected_embed ? "Yes" : "No"; color: auto_detected_embed ? "#6ee7b7" : "#fca5a5" } }
                        Row { spacing: 6; Text { text: "Embed:"; color: theme.textSecondary } Switch { checked: (embed_override === undefined || embed_override === null) ? auto_detected_embed : embed_override; onCheckedChanged: { Project.setEmbedOverride(path, checked); } } }
                    }
                }
            }
        }

        ListModel { id: fontsModel }

    }

    Component.onCompleted: {
        // Auto-load manifest and scan fonts for a smoother experience
        Project.loadManifest("");
        var res = Project.scanFonts();
        fontsModel.clear();
        for (var i=0;i<res.length;i++) {
            fontsModel.append(res[i]);
        }
    }
}
