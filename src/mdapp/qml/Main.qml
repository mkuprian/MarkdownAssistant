// =============================================================================
// Main.qml - Markdown Editor Main Window
// =============================================================================
//
// Split-view layout with:
//   - Left: TextArea for markdown editing
//   - Right: ScrollView with Text for HTML preview (or WebEngineView when available)
//   - Toolbar: New, Load, Save, Render buttons
//
// =============================================================================

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import MdEditor 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 1200
    height: 800
    minimumWidth: 800
    minimumHeight: 600
    title: {
        let name = doc.filePath ? doc.filePath.split('/').pop().split('\\').pop() : "Untitled"
        return (doc.modified ? "* " : "") + name + " - Markdown Editor"
    }

    // -------------------------------------------------------------------------
    // Document Controller
    // -------------------------------------------------------------------------
    DocumentController {
        id: doc

        onPreviewReady: function(html) {
            // Wrap in basic HTML structure with styling
            previewContent.text = wrapHtmlForPreview(html)
        }

        onErrorOccurred: function(message) {
            errorDialog.text = message
            errorDialog.open()
        }
    }

    // -------------------------------------------------------------------------
    // Helper Functions
    // -------------------------------------------------------------------------
    function wrapHtmlForPreview(bodyHtml) {
        // For Text element, we use Qt's rich text subset
        // Strip HTML tags for basic preview, or return as-is for styled display
        return bodyHtml
    }

    // -------------------------------------------------------------------------
    // File Dialogs
    // -------------------------------------------------------------------------
    FileDialog {
        id: openDialog
        title: "Open Markdown File"
        nameFilters: ["Markdown files (*.md *.markdown)", "All files (*)"]
        onAccepted: {
            doc.loadFileUrl(selectedFile)
            doc.renderToHtml()
        }
    }

    FileDialog {
        id: saveDialog
        title: "Save Markdown File"
        nameFilters: ["Markdown files (*.md *.markdown)", "All files (*)"]
        fileMode: FileDialog.SaveFile
        onAccepted: {
            doc.saveFileUrl(selectedFile)
        }
    }

    // -------------------------------------------------------------------------
    // Error Dialog
    // -------------------------------------------------------------------------
    Dialog {
        id: errorDialog
        property alias text: errorLabel.text
        title: "Error"
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent

        Label {
            id: errorLabel
            wrapMode: Text.WordWrap
        }
    }

    // -------------------------------------------------------------------------
    // Unsaved Changes Dialog
    // -------------------------------------------------------------------------
    Dialog {
        id: unsavedChangesDialog
        property var pendingAction: null
        title: "Unsaved Changes"
        modal: true
        standardButtons: Dialog.Save | Dialog.Discard | Dialog.Cancel
        anchors.centerIn: parent

        Label {
            text: "The document has unsaved changes.\nDo you want to save before continuing?"
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            if (doc.filePath) {
                doc.saveFile(doc.filePath)
            } else {
                saveDialog.open()
            }
            if (pendingAction) pendingAction()
        }

        onDiscarded: {
            if (pendingAction) pendingAction()
            close()
        }
    }

    // -------------------------------------------------------------------------
    // Main Layout
    // -------------------------------------------------------------------------
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ---------------------------------------------------------------------
        // Toolbar
        // ---------------------------------------------------------------------
        ToolBar {
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                ToolButton {
                    text: "New"
                    icon.name: "document-new"
                    onClicked: {
                        if (doc.modified) {
                            unsavedChangesDialog.pendingAction = () => doc.newDocument()
                            unsavedChangesDialog.open()
                        } else {
                            doc.newDocument()
                        }
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: "Create a new document (Ctrl+N)"
                }

                ToolButton {
                    text: "Open"
                    icon.name: "document-open"
                    onClicked: {
                        if (doc.modified) {
                            unsavedChangesDialog.pendingAction = () => openDialog.open()
                            unsavedChangesDialog.open()
                        } else {
                            openDialog.open()
                        }
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: "Open a file (Ctrl+O)"
                }

                ToolButton {
                    text: "Save"
                    icon.name: "document-save"
                    onClicked: {
                        if (doc.filePath) {
                            doc.saveFile(doc.filePath)
                        } else {
                            saveDialog.open()
                        }
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: "Save the document (Ctrl+S)"
                }

                ToolSeparator {}

                ToolButton {
                    text: "Render"
                    icon.name: "view-preview"
                    highlighted: true
                    onClicked: doc.renderToHtml()
                    ToolTip.visible: hovered
                    ToolTip.text: "Render markdown to HTML preview (Ctrl+R)"
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: "Parser: " + doc.parserName
                    opacity: 0.6
                }
            }
        }

        // ---------------------------------------------------------------------
        // Split View: Editor + Preview
        // ---------------------------------------------------------------------
        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            // -----------------------------------------------------------------
            // Left Panel: Markdown Editor
            // -----------------------------------------------------------------
            Pane {
                SplitView.preferredWidth: parent.width / 2
                SplitView.minimumWidth: 200
                padding: 0

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Header
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        color: palette.alternateBase

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8

                            Label {
                                text: "Markdown"
                                font.bold: true
                            }

                            Item { Layout.fillWidth: true }

                            Label {
                                text: "Lines: " + (editor.text.split('\n').length)
                                opacity: 0.6
                            }
                        }
                    }

                    // Editor
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        TextArea {
                            id: editor
                            text: doc.text
                            placeholderText: "Enter markdown here..."
                            font.family: "Consolas, Monaco, 'Courier New', monospace"
                            font.pixelSize: 14
                            wrapMode: TextEdit.Wrap
                            selectByMouse: true
                            tabStopDistance: 28

                            onTextChanged: {
                                if (text !== doc.text) {
                                    doc.text = text
                                }
                            }

                            background: Rectangle {
                                color: palette.base
                            }
                        }
                    }
                }
            }

            // -----------------------------------------------------------------
            // Right Panel: HTML Preview
            // -----------------------------------------------------------------
            Pane {
                SplitView.preferredWidth: parent.width / 2
                SplitView.minimumWidth: 200
                padding: 0

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Header
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        color: palette.alternateBase

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8

                            Label {
                                text: "Preview"
                                font.bold: true
                            }

                            Item { Layout.fillWidth: true }

                            Label {
                                text: previewContent.text.length > 0 ? "HTML ready" : "Click Render"
                                opacity: 0.6
                            }
                        }
                    }

                    // Preview (using Text with rich text for now)
                    // When WebEngine is available, this can be replaced with WebEngineView
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true

                        Text {
                            id: previewContent
                            width: parent.width - 32
                            padding: 16
                            textFormat: Text.RichText
                            wrapMode: Text.Wrap
                            font.pixelSize: 14
                            color: palette.text

                            onLinkActivated: function(link) {
                                Qt.openUrlExternally(link)
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.NoButton
                                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                            }
                        }
                    }
                }
            }
        }

        // ---------------------------------------------------------------------
        // Status Bar
        // ---------------------------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            height: 24
            color: palette.alternateBase

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Label {
                    text: doc.filePath ? doc.filePath : "No file loaded"
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                    opacity: 0.7
                }

                Label {
                    text: doc.modified ? "Modified" : "Saved"
                    opacity: 0.7
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // Keyboard Shortcuts
    // -------------------------------------------------------------------------
    Shortcut {
        sequence: "Ctrl+N"
        onActivated: {
            if (doc.modified) {
                unsavedChangesDialog.pendingAction = () => doc.newDocument()
                unsavedChangesDialog.open()
            } else {
                doc.newDocument()
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+O"
        onActivated: {
            if (doc.modified) {
                unsavedChangesDialog.pendingAction = () => openDialog.open()
                unsavedChangesDialog.open()
            } else {
                openDialog.open()
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+S"
        onActivated: {
            if (doc.filePath) {
                doc.saveFile(doc.filePath)
            } else {
                saveDialog.open()
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+R"
        onActivated: doc.renderToHtml()
    }

    // -------------------------------------------------------------------------
    // Startup
    // -------------------------------------------------------------------------
    Component.onCompleted: {
        // Check for command line argument
        if (Qt.application.arguments.length > 1) {
            let filePath = Qt.application.arguments[1]
            doc.loadFile(filePath)
            doc.renderToHtml()
        }
    }
}
