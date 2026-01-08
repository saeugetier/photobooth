import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt.labs.platform
import CollageModel
import FileIO

Item {
    id: collageTemplateEditor
    
    property alias textArea: xmlTextArea
    property string xmlFilePath: ""
    
    FileIO {
        id: fileReader
    }
    
    FileIO {
        id: fileWriter
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
                enabled: xmlFilePath.length > 0
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
        if(path.length > 0) {
            xmlFilePath = path
        } else {
            // Get the AppLocalDataLocation path
            var locations = StandardPaths.standardLocations(StandardPaths.AppLocalDataLocation)
            if(locations.length > 0) {
                xmlFilePath = locations[0] + "/Collages.xml"
            }
        }
        loadXmlFile()
    }
    
    function loadXmlFile() {
        if(xmlFilePath.length === 0) {
            statusLabel.text = qsTr("Error: No file path specified")
            statusLabel.color = "#F44336"
            return
        }
        
        // Try to read the custom file
        fileReader.source = xmlFilePath
        var content = fileReader.read()
        
        if(content.length === 0) {
            // Try to load from resources as template
            fileReader.source = "qrc:/XmlData.xml"
            var defaultContent = fileReader.read()
            if(defaultContent.length > 0) {
                xmlTextArea.text = defaultContent
                statusLabel.text = qsTr("Loaded default template")
                statusLabel.color = "#FF9800"
            } else {
                statusLabel.text = qsTr("Error loading file")
                statusLabel.color = "#F44336"
            }
        } else {
            xmlTextArea.text = content
            statusLabel.text = qsTr("Loaded successfully")
            statusLabel.color = "#4CAF50"
        }
        
        // Clear status after 3 seconds
        statusTimer.restart()
    }
    
    function saveXmlFile() {
        if(xmlFilePath.length === 0) {
            statusLabel.text = qsTr("Error: No file path specified")
            statusLabel.color = "#F44336"
            return
        }
        
        fileWriter.source = xmlFilePath
        var success = fileWriter.write(xmlTextArea.text)
        if(success) {
            statusLabel.text = qsTr("Saved successfully!")
            statusLabel.color = "#4CAF50"
        } else {
            statusLabel.text = qsTr("Error saving file")
            statusLabel.color = "#F44336"
        }
        
        // Clear status after 3 seconds
        statusTimer.restart()
    }
    
    function resetToDefault() {
        fileReader.source = "qrc:/XmlData.xml"
        var defaultContent = fileReader.read()
        if(defaultContent.length > 0) {
            xmlTextArea.text = defaultContent
            statusLabel.text = qsTr("Reset to default template")
            statusLabel.color = "#4CAF50"
        } else {
            statusLabel.text = qsTr("Error loading default template")
            statusLabel.color = "#F44336"
        }
        
        // Clear status after 3 seconds
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
