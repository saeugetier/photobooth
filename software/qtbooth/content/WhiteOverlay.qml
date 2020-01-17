import QtQuick 2.0

Rectangle {
    id: rect
    color: "white"
    opacity: 0.0

    states: [
        State {
            name: "triggered"
            PropertyChanges {
                target: rect
                opacity: 1.0
            }
        },
        State {
            name: "released"
            PropertyChanges {
                target: rect
                opacity: 0.0
            }
        }

    ]

    transitions: [
        Transition {
            NumberAnimation { property: "opacity"; duration: 300 }
        }
    ]
}
