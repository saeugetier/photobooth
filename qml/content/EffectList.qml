import QtQuick

ListModel {
    id: sources
    ListElement { name: "No effect"; effectText: qsTr("No effect"); effectSource: "qrc:/shaders/passthrough.frag.qsb" }
    ListElement { name: "Black & White"; effectText: qsTr("Black & White"); effectSource: "qrc:/shaders/bw.frag.qsb" }
    ListElement { name: "Greyscale"; effectText: qsTr("Greyscale"); effectSource: "qrc:/shaders/greyscale.frag.qsb" }
    ListElement { name: "Posterize"; effectText: qsTr("Posterize"); effectSource: "qrc:/shaders/posterize.frag.qsb" }
    ListElement { name: "Fish Eye"; effectText: qsTr("Fish Eye"); effectSource: "qrc:/shaders/fisheye.frag.qsb" }
    ListElement { name: "Pop Art"; effectText: qsTr("Pop Art"); effectSource: "qrc:/shaders/warhol.frag.qsb" }
    ListElement { name: "Sepia"; effectText: qsTr("Sepia"); effectSource: "qrc:/shaders/sepia.frag.qsb" }
    ListElement { name: "Vignette"; effectText: qsTr("Vignette"); effectSource: "qrc:/shaders/vignette.frag.qsb" }
    ListElement { name: "Green Seeker"; effectText: qsTr("Green Seeker"); effectSource: "qrc:/shaders/greenseeker.frag.qsb" }
    ListElement { name: "Blue Seeker"; effectText: qsTr("Blue Seeker"); effectSource: "qrc:/shaders/blueseeker.frag.qsb" }
    ListElement { name: "Red Seeker"; effectText: qsTr("Red Seeker"); effectSource: "qrc:/shaders/redseeker.frag.qsb" }
    ListElement { name: "Yellow Seeker"; effectText: qsTr("Yellow Seeker"); effectSource: "qrc:/shaders/yellowseeker.frag.qsb" }
}
