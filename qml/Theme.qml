import QtQuick 2.0
QtObject {
    id: theme
    property string currentTheme: "dark"

    // General
    property color background: currentTheme === "dark" ? "#111215" : "#f6f6f6"
    property color header: currentTheme === "dark" ? "#0f1720" : "#f0f0f2"
    property color panel: currentTheme === "dark" ? "#0b0f14" : "#ffffff"
    property color panelBorder: currentTheme === "dark" ? "#1b2430" : "#d7dfe6"

    // Text
    property color textPrimary: currentTheme === "dark" ? "#e6eef6" : "#0b1720"
    property color textSecondary: currentTheme === "dark" ? "#9fb0c9" : "#5b6b7a"

    // Accent
    property color accent: "#1f6feb"

    // Page specific
    property color pageBackground: currentTheme === "dark" ? "#fffaf6" : "#fffdf9"
    property color pageBorder: currentTheme === "dark" ? "#ddd2c8" : "#d1c6bb"
    property color previewBg: currentTheme === "dark" ? "#0b1014" : "#f2f6f8"
}
