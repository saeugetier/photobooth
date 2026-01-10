import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform
import QtCore
import FileIO
import CollageModel

Item {
    id: collageTemplateEditor
    
    readonly property string defaultXmlPath: "qrc:/XmlData.xml"
    property string xmlFilePath: ""
    property var collageTemplates: []
    property int currentTemplateIndex: -1
    property var currentTemplate: null
    
    FileIO {
        id: fileIO
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
        loadTemplates()
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // Left panel: Template list
        ColumnLayout {
            Layout.preferredWidth: 250
            Layout.fillHeight: true
            spacing: 5
            
            Label {
                text: qsTr("Collage Templates")
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }
            
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                ListView {
                    id: templateListView
                    clip: true
                    model: collageTemplates.length
                    delegate: ItemDelegate {
                        width: ListView.view.width
                        text: index < collageTemplates.length ? collageTemplates[index].name : ""
                        highlighted: index === currentTemplateIndex
                        onClicked: {
                            currentTemplateIndex = index
                            currentTemplate = collageTemplates[index]
                        }
                    }
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 5
                
                Button {
                    text: qsTr("Add")
                    Layout.fillWidth: true
                    onClicked: addNewTemplate()
                }
                
                Button {
                    text: qsTr("Delete")
                    Layout.fillWidth: true
                    enabled: currentTemplateIndex >= 0
                    onClicked: deleteCurrentTemplate()
                }
            }
            
            Button {
                text: qsTr("Save All")
                Layout.fillWidth: true
                onClicked: saveTemplates()
            }
            
            Label {
                id: statusLabel
                text: ""
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                font.pixelSize: 10
            }
        }
        
        // Right panel: Template editor
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            
            Label {
                text: currentTemplate ? qsTr("Edit Template: ") + currentTemplate.name : qsTr("Select a template to edit")
                font.pixelSize: 16
                font.bold: true
            }
            
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: currentTemplate !== null
                
                ColumnLayout {
                    width: parent.parent.width - 20
                    spacing: 10
                    
                    // Basic properties
                    GroupBox {
                        title: qsTr("Basic Properties")
                        Layout.fillWidth: true
                        
                        GridLayout {
                            columns: 2
                            rowSpacing: 5
                            columnSpacing: 10
                            anchors.fill: parent
                            
                            Label { text: qsTr("Name:") }
                            TextField {
                                id: nameField
                                Layout.fillWidth: true
                                text: currentTemplate ? currentTemplate.name : ""
                                onEditingFinished: {
                                    if(currentTemplate) {
                                        currentTemplate.name = text
                                        collageTemplates[currentTemplateIndex] = currentTemplate
                                        templateListView.model = 0
                                        templateListView.model = collageTemplates.length
                                    }
                                }
                            }
                            
                            Label { text: qsTr("Icon:") }
                            TextField {
                                id: iconField
                                Layout.fillWidth: true
                                text: currentTemplate ? currentTemplate.icon : ""
                                onEditingFinished: {
                                    if(currentTemplate) currentTemplate.icon = text
                                }
                            }
                            
                            Label { text: qsTr("Printable:") }
                            CheckBox {
                                id: printableCheck
                                checked: currentTemplate ? (currentTemplate.printable !== false) : true
                                onToggled: {
                                    if(currentTemplate) currentTemplate.printable = checked
                                }
                            }
                        }
                    }
                    
                    // Images section
                    GroupBox {
                        title: qsTr("Images")
                        Layout.fillWidth: true
                        
                        GridLayout {
                            columns: 2
                            rowSpacing: 5
                            columnSpacing: 10
                            anchors.fill: parent
                            
                            Label { text: qsTr("Background:") }
                            TextField {
                                id: backgroundField
                                Layout.fillWidth: true
                                text: currentTemplate ? currentTemplate.background : ""
                                onEditingFinished: {
                                    if(currentTemplate) currentTemplate.background = text
                                }
                            }
                            
                            Label { text: qsTr("Foreground:") }
                            TextField {
                                id: foregroundField
                                Layout.fillWidth: true
                                text: currentTemplate ? currentTemplate.foreground : ""
                                onEditingFinished: {
                                    if(currentTemplate) currentTemplate.foreground = text
                                }
                            }
                        }
                    }
                    
                    // Size section
                    GroupBox {
                        title: qsTr("Collage Size")
                        Layout.fillWidth: true
                        
                        GridLayout {
                            columns: 2
                            rowSpacing: 5
                            columnSpacing: 10
                            anchors.fill: parent
                            
                            Label { text: qsTr("Width:") }
                            SpinBox {
                                id: widthSpin
                                from: 100
                                to: 10000
                                value: currentTemplate ? currentTemplate.width : 3570
                                onValueModified: {
                                    if(currentTemplate) currentTemplate.width = value
                                }
                            }
                            
                            Label { text: qsTr("Height:") }
                            SpinBox {
                                id: heightSpin
                                from: 100
                                to: 10000
                                value: currentTemplate ? currentTemplate.height : 2380
                                onValueModified: {
                                    if(currentTemplate) currentTemplate.height = value
                                }
                            }
                        }
                    }
                    
                    // Photo slots
                    GroupBox {
                        title: qsTr("Photo Slots")
                        Layout.fillWidth: true
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 5
                            
                            ListView {
                                id: photoSlotsView
                                Layout.fillWidth: true
                                Layout.preferredHeight: Math.min(contentHeight, 300)
                                clip: true
                                model: currentTemplate ? currentTemplate.images.length : 0
                                
                                delegate: ItemDelegate {
                                    width: ListView.view.width
                                    text: qsTr("Slot %1").arg(index + 1)
                                    onClicked: {
                                        photoSlotEditor.currentSlotIndex = index
                                        photoSlotEditor.visible = true
                                    }
                                }
                            }
                            
                            RowLayout {
                                Layout.fillWidth: true
                                
                                Button {
                                    text: qsTr("Add Slot")
                                    Layout.fillWidth: true
                                    onClicked: addPhotoSlot()
                                }
                                
                                Button {
                                    text: qsTr("Remove Slot")
                                    Layout.fillWidth: true
                                    enabled: currentTemplate && currentTemplate.images.length > 0
                                    onClicked: removePhotoSlot()
                                }
                            }
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
            
            // Empty state
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: currentTemplate === null
                
                Label {
                    anchors.centerIn: parent
                    text: qsTr("Select a template from the list to edit")
                    font.pixelSize: 16
                    color: "#999999"
                }
            }
        }
    }
    
    // Photo slot editor dialog
    Dialog {
        id: photoSlotEditor
        property int currentSlotIndex: -1
        
        title: qsTr("Edit Photo Slot")
        modal: true
        anchors.centerIn: parent
        width: Math.min(500, parent.width - 40)
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            
            GridLayout {
                columns: 2
                rowSpacing: 5
                columnSpacing: 10
                Layout.fillWidth: true
                
                Label { text: qsTr("Position X (0-1):") }
                SpinBox {
                    id: posXSpin
                    from: 0
                    to: 100
                    value: getSlotValue("x", 0) * 100
                    onValueModified: setSlotValue("x", value / 100.0)
                }
                
                Label { text: qsTr("Position Y (0-1):") }
                SpinBox {
                    id: posYSpin
                    from: 0
                    to: 100
                    value: getSlotValue("y", 0) * 100
                    onValueModified: setSlotValue("y", value / 100.0)
                }
                
                Label { text: qsTr("Width (0-1):") }
                SpinBox {
                    id: sizeWSpin
                    from: 1
                    to: 100
                    value: getSlotValue("width", 100) * 100
                    onValueModified: setSlotValue("width", value / 100.0)
                }
                
                Label { text: qsTr("Height (0-1):") }
                SpinBox {
                    id: sizeHSpin
                    from: 1
                    to: 100
                    value: getSlotValue("height", 100) * 100
                    onValueModified: setSlotValue("height", value / 100.0)
                }
                
                Label { text: qsTr("Rotation (degrees):") }
                SpinBox {
                    id: rotationSpin
                    from: -180
                    to: 180
                    value: getSlotValue("rotation", 0)
                    onValueModified: setSlotValue("rotation", value)
                }
                
                Label { text: qsTr("Border Image:") }
                TextField {
                    id: borderField
                    Layout.fillWidth: true
                    text: getSlotBorderValue("file", "")
                    onEditingFinished: setSlotBorderValue("file", text)
                }
                
                Label { text: qsTr("Effect Preset:") }
                TextField {
                    id: effectPresetField
                    Layout.fillWidth: true
                    text: getSlotValue("effectPreset", "")
                    onEditingFinished: setSlotValue("effectPreset", text)
                }
                
                Label { text: qsTr("Effect Selectable:") }
                CheckBox {
                    id: effectSelectableCheck
                    checked: getSlotValue("effectSelectable", true)
                    onToggled: setSlotValue("effectSelectable", checked)
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                
                Button {
                    text: qsTr("Close")
                    Layout.fillWidth: true
                    onClicked: photoSlotEditor.close()
                }
            }
        }
        
        function getSlotValue(key, defaultValue) {
            if(!currentTemplate || currentSlotIndex < 0 || currentSlotIndex >= currentTemplate.images.length)
                return defaultValue
            var slot = currentTemplate.images[currentSlotIndex]
            return slot[key] !== undefined ? slot[key] : defaultValue
        }
        
        function setSlotValue(key, value) {
            if(!currentTemplate || currentSlotIndex < 0 || currentSlotIndex >= currentTemplate.images.length)
                return
            currentTemplate.images[currentSlotIndex][key] = value
        }
        
        function getSlotBorderValue(key, defaultValue) {
            if(!currentTemplate || currentSlotIndex < 0 || currentSlotIndex >= currentTemplate.images.length)
                return defaultValue
            var slot = currentTemplate.images[currentSlotIndex]
            if(!slot.border) return defaultValue
            return slot.border[key] !== undefined ? slot.border[key] : defaultValue
        }
        
        function setSlotBorderValue(key, value) {
            if(!currentTemplate || currentSlotIndex < 0 || currentSlotIndex >= currentTemplate.images.length)
                return
            if(!currentTemplate.images[currentSlotIndex].border) {
                currentTemplate.images[currentSlotIndex].border = {}
            }
            currentTemplate.images[currentSlotIndex].border[key] = value
        }
    }
    
    function loadTemplates() {
        fileIO.source = xmlFilePath
        var content = String(fileIO.read())
        
        if(content.length === 0) {
            fileIO.source = defaultXmlPath
            content = String(fileIO.read())
        }
        
        if(content.length === 0) {
            showStatus(qsTr("Error loading templates"), "#F44336")
            return
        }
        
        collageTemplates = parseXML(content)
        templateListView.model = collageTemplates.length
        showStatus(qsTr("Loaded %1 templates").arg(collageTemplates.length), "#4CAF50")
    }
    
    function parseXML(xmlContent) {
        var templates = []
        
        // Simple XML parsing
        var collageRegex = /<collage>([\s\S]*?)<\/collage>/g
        var match
        
        while ((match = collageRegex.exec(xmlContent)) !== null) {
            var collageXml = match[1]
            var template = {}
            
            // Parse basic fields
            template.name = extractTag(collageXml, "name")
            template.icon = extractTag(collageXml, "icon")
            template.background = extractTag(collageXml, "background")
            template.foreground = extractTag(collageXml, "foreground")
            template.printable = extractTag(collageXml, "printable") !== "false"
            
            // Parse size
            var sizeMatch = /<size\s+width="(\d+)"\s+height="(\d+)"/.exec(collageXml)
            if(sizeMatch) {
                template.width = parseInt(sizeMatch[1])
                template.height = parseInt(sizeMatch[2])
            } else {
                template.width = 3570
                template.height = 2380
            }
            
            // Parse images
            template.images = []
            var imagesXml = /<images>([\s\S]*?)<\/images>/.exec(collageXml)
            if(imagesXml) {
                var imageRegex = /<image>([\s\S]*?)<\/image>/g
                var imageMatch
                while ((imageMatch = imageRegex.exec(imagesXml[1])) !== null) {
                    var imageXml = imageMatch[1]
                    var image = {}
                    
                    // Position
                    var posMatch = /<position\s+x="([^"]+)"\s+y="([^"]+)"/.exec(imageXml)
                    if(posMatch) {
                        image.x = parseFloat(posMatch[1])
                        image.y = parseFloat(posMatch[2])
                    }
                    
                    // Size
                    var sizeMatch2 = /<size\s+width="([^"]+)"\s+height="([^"]+)"/.exec(imageXml)
                    if(sizeMatch2) {
                        image.width = parseFloat(sizeMatch2[1])
                        image.height = parseFloat(sizeMatch2[2])
                    }
                    
                    // Rotation
                    image.rotation = parseFloat(extractTag(imageXml, "rotation") || "0")
                    
                    // Effect
                    image.effectPreset = extractTag(imageXml, "effectPreset")
                    var effectSelectable = extractTag(imageXml, "effectSelectable")
                    image.effectSelectable = effectSelectable !== "false"
                    
                    // Border
                    var borderXml = /<border>([\s\S]*?)<\/border>/.exec(imageXml)
                    if(borderXml) {
                        image.border = {
                            file: extractTag(borderXml[1], "file")
                        }
                    }
                    
                    template.images.push(image)
                }
            }
            
            templates.push(template)
        }
        
        return templates
    }
    
    function extractTag(xml, tagName) {
        var regex = new RegExp("<" + tagName + ">([^<]*)<\/" + tagName + ">")
        var match = regex.exec(xml)
        return match ? match[1] : ""
    }
    
    function generateXML() {
        var xml = '<?xml version="1.0" encoding="UTF-8"?>\n'
        xml += '<catalog version="1.0">\n'
        
        for(var i = 0; i < collageTemplates.length; i++) {
            var t = collageTemplates[i]
            xml += '    <collage>\n'
            xml += '        <name>' + t.name + '</name>\n'
            if(t.printable === false) {
                xml += '        <printable>false</printable>\n'
            }
            xml += '        <icon>' + t.icon + '</icon>\n'
            xml += '        <background>' + t.background + '</background>\n'
            if(t.foreground) {
                xml += '        <foreground>' + t.foreground + '</foreground>\n'
            }
            xml += '        <size width="' + t.width + '" height="' + t.height + '"/>\n'
            xml += '        <images>\n'
            
            for(var j = 0; j < t.images.length; j++) {
                var img = t.images[j]
                xml += '            <image>\n'
                xml += '                <position x="' + img.x + '" y="' + img.y + '"/>'
                xml += '<size width="' + img.width + '" height="' + img.height + '"/>\n'
                if(img.rotation) {
                    xml += '                <rotation>' + img.rotation + '</rotation>\n'
                }
                if(img.effectPreset) {
                    xml += '                <effectPreset>' + img.effectPreset + '</effectPreset>\n'
                }
                if(img.effectSelectable === false) {
                    xml += '                <effectSelectable>false</effectSelectable>\n'
                }
                if(img.border && img.border.file) {
                    xml += '                <border><file>' + img.border.file + '</file></border>\n'
                }
                xml += '            </image>\n'
            }
            
            xml += '        </images>\n'
            xml += '    </collage>\n'
        }
        
        xml += '</catalog>\n'
        return xml
    }
    
    function saveTemplates() {
        var xml = generateXML()
        fileIO.source = xmlFilePath
        var success = fileIO.write(xml)
        if(success) {
            showStatus(qsTr("Saved successfully!"), "#4CAF50")
        } else {
            showStatus(qsTr("Error saving file"), "#F44336")
        }
    }
    
    function addNewTemplate() {
        var template = {
            name: "New Template",
            icon: "Unknown.svg",
            background: "WhiteBackground.png",
            foreground: "",
            printable: true,
            width: 3570,
            height: 2380,
            images: [
                { x: 0.0, y: 0.0, width: 1.0, height: 1.0, rotation: 0 }
            ]
        }
        collageTemplates.push(template)
        templateListView.model = collageTemplates.length
        currentTemplateIndex = collageTemplates.length - 1
        currentTemplate = template
        showStatus(qsTr("Added new template"), "#4CAF50")
    }
    
    function deleteCurrentTemplate() {
        if(currentTemplateIndex < 0) return
        
        collageTemplates.splice(currentTemplateIndex, 1)
        templateListView.model = collageTemplates.length
        
        if(currentTemplateIndex >= collageTemplates.length) {
            currentTemplateIndex = collageTemplates.length - 1
        }
        
        if(currentTemplateIndex >= 0) {
            currentTemplate = collageTemplates[currentTemplateIndex]
        } else {
            currentTemplate = null
        }
        
        showStatus(qsTr("Template deleted"), "#4CAF50")
    }
    
    function addPhotoSlot() {
        if(!currentTemplate) return
        
        currentTemplate.images.push({
            x: 0.0,
            y: 0.0,
            width: 0.5,
            height: 0.5,
            rotation: 0
        })
        photoSlotsView.model = currentTemplate.images.length
        showStatus(qsTr("Added photo slot"), "#4CAF50")
    }
    
    function removePhotoSlot() {
        if(!currentTemplate || currentTemplate.images.length === 0) return
        
        currentTemplate.images.pop()
        photoSlotsView.model = currentTemplate.images.length
        showStatus(qsTr("Removed photo slot"), "#4CAF50")
    }
    
    function showStatus(text, color) {
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
