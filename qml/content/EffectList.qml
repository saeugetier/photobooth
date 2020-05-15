import QtQuick 2.0

ListModel {
    id: sources
    ListElement { name: "No effect"; effectText: qsTr("No effect"); effectSource: "passthrough.fsh" }
    ListElement { name: "Black & White"; effectText: qsTr("Black & White"); effectSource: "bw.fsh" }
    ListElement { name: "Greyscale"; effectText: qsTr("Greyscale"); effectSource: "greyscale.fsh" }
    ListElement { name: "Posterize"; effectText: qsTr("Posterize"); effectSource: "posterize.fsh" }
    ListElement { name: "Fish Eye"; effectText: qsTr("Fish Eye"); effectSource: "fisheye.fsh" }
    ListElement { name: "Pop Art"; effectText: qsTr("Pop Art"); effectSource: "warhol.fsh" }
    ListElement { name: "Sepia"; effectText: qsTr("Sepia"); effectSource: "sepia.fsh" }
    ListElement { name: "Vignette"; effectText: qsTr("Vignette"); effectSource: "vignette.fsh" }
    ListElement { name: "Green Seeker"; effectText: qsTr("Green Seeker"); effectSource: "greenseeker.fsh" }
    ListElement { name: "Blue Seeker"; effectText: qsTr("Blue Seeker"); effectSource: "blueseeker.fsh" }
    ListElement { name: "Red Seeker"; effectText: qsTr("Red Seeker"); effectSource: "redseeker.fsh" }
    ListElement { name: "Yellow Seeker"; effectText: qsTr("Yellow Seeker"); effectSource: "yellowseeker.fsh" }
}
