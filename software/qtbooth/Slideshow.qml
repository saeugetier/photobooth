import QtQuick 2.0
import QtQuick.XmlListModel 2.0
import QtQml.Models 2.1

Rectangle {
    id: root

    property Item displayItem: null

    width: 300; height: 400

    color: "black"

    DelegateModel {
        id: visualModel

        model: XmlListModel {
            source: "http://api.flickr.com/services/feeds/photos_public.gne?format=rss2"
            query: "/rss/channel/item"
            namespaceDeclarations: "declare namespace media=\"http://search.yahoo.com/mrss/\";"

            XmlRole { name: "imagePath"; query: "media:thumbnail/@url/string()" }
            XmlRole { name: "url"; query: "media:content/@url/string()" }
        }

        delegate: Item {
            id: delegateItem

            width: 76; height: 76

            Rectangle {
                id: image
                x: 0; y: 0; width: 76; height: 76
                border.width: 1
                border.color: "white"
                color: "black"

                Image {
                    anchors.fill: parent
                    anchors.leftMargin: 1
                    anchors.topMargin: 1

                    source: imagePath
                    fillMode: Image.PreserveAspectFit

                }

                MouseArea {
                    id: clickArea
                    anchors.fill: parent

                    onClicked: root.displayItem = root.displayItem !== delegateItem ? delegateItem : null
                }

                states: [
                    State {
                        when: root.displayItem === delegateItem
                        name: "inDisplay";
                        ParentChange { target: image; parent: imageContainer; x: 75; y: 75; width: 150; height: 150 }
                        PropertyChanges { target: image; z: 2 }
                        PropertyChanges { target: delegateItem; DelegateModel.inItems: false }
                    },
                    State {
                        when: root.displayItem !== delegateItem
                        name: "inList";
                        ParentChange { target: image; parent: delegateItem; x: 2; y: 2; width: 75; height: 75 }
                        PropertyChanges { target: image; z: 1 }
                        PropertyChanges { target: delegateItem; DelegateModel.inItems: true }
                    }
                ]

                transitions: [
                    Transition {
                        from: "inList"
                        SequentialAnimation {
                            PropertyAction { target: delegateItem; property: "DelegateModel.inPersistedItems"; value: true }
                            ParentAnimation {
                                target: image;
                                via: root
                                NumberAnimation { target: image; properties: "x,y,width,height"; duration: 1000 }
                            }
                        }
                    }, Transition {
                        from: "inDisplay"
                        SequentialAnimation {
                            ParentAnimation {
                                target: image
                                NumberAnimation { target: image; properties: "x,y,width,height"; duration: 1000 }
                            }
                            PropertyAction { target: delegateItem; property: "DelegateModel.inPersistedItems"; value: false }
                        }
                    }
                ]
            }
        }
    }

    PathView {
        id: imagePath

        anchors { left: parent.left; top: imageContainer.bottom; right: parent.right; bottom: parent.bottom }
        model: visualModel

        pathItemCount: 7
        path: Path {
            startX: -50; startY: 0
            PathQuad { x: 150; y: 50; controlX: 0; controlY: 50 }
            PathQuad { x: 350; y: 0; controlX: 300; controlY: 50 }
        }
    }

    Item {
        id: imageContainer
        anchors { fill: parent; bottomMargin: 100 }
    }
}
