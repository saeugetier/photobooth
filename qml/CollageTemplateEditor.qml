import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform
import QtCore
import FileIO

Item {
    id: collageTemplateEditor
    
    readonly property string defaultXmlPath: "qrc:/XmlData.xml"
    property alias textArea: xmlTextArea
    property string xmlFilePath: ""
    
    FileIO {
        id: fileIO
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        Label {
            text: qsTr("Collage Template Manager")
            font.pixelSize: 18
            font.bold: true
        }
        
        Label {
            text: qsTr("Edit XML configuration for collage layouts:")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Label {
                text: qsTr("XML File:")
            }
            
            Label {
                id: filePathLabel
                text: xmlFilePath
                Layout.fillWidth: true
                elide: Text.ElideLeft
            }
            
            Button {
                text: qsTr("Reload")
                onClicked: loadXmlFile()
            }
        }
        
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            TextArea {
                id: xmlTextArea
                font.family: "Monospace"
                font.pixelSize: 12
                wrapMode: TextArea.NoWrap
                selectByMouse: true
                placeholderText: qsTr("XML content will appear here...")
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Button {
                text: qsTr("Save")
                enabled: xmlFilePath.length !== 0
                onClicked: saveXmlFile()
            }
            
            Button {
                text: qsTr("Reset to Default")
                onClicked: resetToDefault()
            }
            
            Item {
                Layout.fillWidth: true
            }
            
            Label {
                id: statusLabel
                text: ""
                color: "#4CAF50"
            }
        }
        
        Label {
            text: qsTr("Note: Changes will take effect after restarting the application or reloading templates.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.italic: true
            color: "#999999"
        }
    }
    
    Component.onCompleted: {
        // Try to find the collages file
        var path = StandardPaths.locate(StandardPaths.AppLocalDataLocation, "Collages.xml")
        if(path.length !== 0) {
            xmlFilePath = path
        } else {
            // Get the AppLocalDataLocation path
            var locations = StandardPaths.standardLocations(StandardPaths.AppLocalDataLocation)
            if(locations.length !== 0) {
                xmlFilePath = locations[0] + "/Collages.xml"
            }
        }
        loadXmlFile()
    }
    
    function loadXmlFile() {
        if(xmlFilePath.length === 0) {
            showStatusMessage(qsTr("Error: No file path specified"), "#F44336")
            return
        }
        
        // Try to read the custom file
        fileIO.source = xmlFilePath
        var content = String(fileIO.read())
        
        if(content.length === 0) {
            // Try to load from resources as template
            fileIO.source = defaultXmlPath
            var defaultContent = String(fileIO.read())
            if(defaultContent.length !== 0) {
                xmlTextArea.text = defaultContent
                showStatusMessage(qsTr("Loaded default template"), "#FF9800")
            } else {
                showStatusMessage(qsTr("Error loading file"), "#F44336")
            }
        } else {
            xmlTextArea.text = content
            showStatusMessage(qsTr("Loaded successfully"), "#4CAF50")
        }
    }
    
    function saveXmlFile() {
        if(xmlFilePath.length === 0) {
            showStatusMessage(qsTr("Error: No file path specified"), "#F44336")
            return
        }
        
        fileIO.source = xmlFilePath
        var success = fileIO.write(xmlTextArea.text)
        if(success) {
            showStatusMessage(qsTr("Saved successfully!"), "#4CAF50")
        } else {
            showStatusMessage(qsTr("Error saving file"), "#F44336")
        }
    }
    
    function resetToDefault() {
        fileIO.source = defaultXmlPath
        var defaultContent = String(fileIO.read())
        if(defaultContent.length !== 0) {
            xmlTextArea.text = defaultContent
            showStatusMessage(qsTr("Reset to default template"), "#4CAF50")
        } else {
            showStatusMessage(qsTr("Error loading default template"), "#F44336")
        }
    }
    
    function showStatusMessage(text, color) {
        statusLabel.text = text
        statusLabel.color = color
        statusTimer.restart()
    }
    
    Timer {
        id: statusTimer
        interval: 3000
        running: false
        repeat: false
        onTriggered: {
            statusLabel.text = ""
        }
    }
}
