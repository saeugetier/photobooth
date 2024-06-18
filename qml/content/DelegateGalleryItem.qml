import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQml 2.14
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.0
import "../scripts/utils.js" as Script
import "../styles" as Style

Package {

    signal printImage(string filename)

    Item { Package.name: 'grid'
        id: gridDelegate
        width: root.width; height: root.height
    }
    Item { Package.name: 'list'
        id: fullDelegate
        width: root.width; height: root.height
    }

// ===================================================

    Item {
        width: 160; height: 153

        Item {
            id: photoWrapper

            x: 0; y: 0
            width: 280; height: 266

            BorderImage {
                anchors.fill: placeHolder
                anchors {
                    leftMargin: -15
                    topMargin: -10
                    rightMargin: -15
                    bottomMargin: -15
                }
                source: '../../images/polaroid.svg.png'
                border {
                    top: 25
                    left: 25
                    right: 25
                    bottom: 65
                }
            }
            Rectangle {
                id: placeHolder

                property int w: 280
                property int h: 180
                property double s: Script.calculateScale(w, h, photoWrapper.width)

                color: 'white'
                anchors.centerIn: parent
                antialiasing: true
                width:  w * s; height: h * s; z: -1
                visible: originalImage.status != Image.Ready
                Rectangle {
                    color: "#878787"
                    antialiasing: true
                    anchors {
                        fill: parent
                        topMargin: 3
                        bottomMargin: 3
                        leftMargin: 3
                        rightMargin: 3
                    }
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: originalImage.status != Image.Ready
            }
            Image {
                id: originalImage

                sourceSize.width: 256  //save memory
                sourceSize.height: 256

                asynchronous: true
                cache: true
                anchors.fill: photoWrapper
                antialiasing: true
                source: foldermodel.folder + "/" + fileName

                fillMode: Image.PreserveAspectFit
            }
            ProgressBar {
                anchors.centerIn: parent
                visible: hqImage.status == Image.Loading
                value: hqImage.progress
                //style: Style.RoundProgressBarStyle {}
            }
            Image {
                id: hqImage

                sourceSize.width: 1024
                sourceSize.height: 1024

                asynchronous: true
                cache: false
                anchors.fill: photoWrapper
                antialiasing: true
                source: ""
                visible: false
                fillMode: Image.PreserveAspectFit

                onSourceChanged: {
                    console.log("hqImage source:", source)
                }

                Row {
                    id: printButtonRow
                    anchors.right: parent.right
                    anchors.rightMargin: 20
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 20

                    spacing: 10

                    layer.enabled: true
                    layer.effect: Glow {
                        color: "black"
                        samples: 20
                        spread: 0.3
                    }

                    Text {
                        id: textLabel
                        color: "#ffffff"
                        text: !galleryForm.printer.busy ? qsTr("Print") : qsTr("Printer busy")
                        font.family: "DejaVu Serif"
                        wrapMode: Text.WrapAnywhere
                        font.pixelSize: 64
                        font.capitalization: Font.AllUppercase
                    }

                    ToolButton {
                        id: printButton
                        text: "\uE802" // icon-print
                        font.family: "fontello"
                        font.pixelSize: 64
                        enabled: !galleryForm.printer.busy

                        scale: hovered ? 1.1 : 1

                        layer.enabled: true
                        layer.effect: Glow {
                            color: "black"
                            samples: 20
                            spread: 0.3
                        }

                        onClicked:
                        {
                            console.log("Print image: " + hqImage.source)
                            printImage(hqImage.source)
                            root.state = 'inGrid'
                            galleryForm.exitGallery()
                        }
                    }
                }

                ToolButton {
                    id: closeButton
                    anchors.right: hqImage.right
                    anchors.top: hqImage.top
                    anchors.rightMargin: 20
                    anchors.topMargin: 20
                    text: "\uE81F"  // cancel button
                    font.family: "fontello"
                    font.pixelSize: 64
                    enabled: true

                    Material.foreground: Material.Red

                    opacity: 0.7

                    onClicked:
                    {

                        root.state = 'inGrid'
                    }
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: hqImage.status != Image.Ready
                }

            }

            Binding {
                target: root
                property: "imageLoading"
                value: (hqImage.status == Image.Loading) ? 1 : 0;
                when: fullDelegate.ListView.isCurrentItem
                restoreMode: Binding.RestoreBinding
            }

            MouseArea {
                anchors.fill: photoWrapper
                z: root.state == 'inGrid' ? 0 : -1
                onClicked: {
                    gridDelegate.GridView.view.currentIndex = index;
                    if (root.state == 'inGrid') {
                        root.state = 'fullscreen'
                    }
                }
            }

            states: [
                State {
                    name: 'inGrid'; when: root.state == 'inGrid' || root.state == ''
                    ParentChange {
                        target: photoWrapper
                        parent: gridDelegate;
                        x: 10; y: 0
                    }
                    PropertyChanges {
                        target: exitButton
                        visible: true
                    }
                },
                State {
                    name: 'fullscreen'; when: root.state == 'fullscreen'
                    ParentChange {
                        target: photoWrapper
                        parent: fullDelegate
                        x: 0; y: 0;
                        width: root.width; height: root.height
                    }
                    PropertyChanges {
                        target: hqImage
                        source: gridDelegate.GridView.isCurrentItem
                                ? foldermodel.folder + "/" + fileName
                                : ""
                        visible: true
                    }
                    PropertyChanges {
                        target: exitButton
                        visible: false
                    }
                }
            ]

            onStateChanged: {
                if(state == 'fullscreen' && gridDelegate.GridView.isCurrentItem)
                    console.log("State changed to 'fullscreen' for '"+fileName+"' record")
            }

            transitions: [
                Transition {
                    from: 'inGrid'; to: 'fullscreen'
                    SequentialAnimation {
                        PauseAnimation {
                            duration: gridDelegate.GridView.isCurrentItem ? 0 : 600
                        }
                        ParentAnimation {
                            target: photoWrapper; via: foreground
                            NumberAnimation {
                                targets: photoWrapper
                                properties: 'x,y,width,height,opacity'
                                duration: gridDelegate.GridView.isCurrentItem ? 600 : 1
                                easing.type: Easing.OutQuart
                            }
                        }
                    }
                },
                Transition {
                    from: 'fullscreen'; to: 'inGrid'
                    ParentAnimation {
                        target: photoWrapper; via: foreground
                        NumberAnimation {
                            targets: photoWrapper
                            properties: 'x,y,width,height,opacity'
                            duration: gridDelegate.GridView.isCurrentItem ? 600 : 1
                            easing.type: Easing.OutQuart
                        }
                    }
                }
            ]
        }
    }
}
