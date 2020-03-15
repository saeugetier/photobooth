import QtQuick 2.5

Item {
    id: countDown
    signal timeout
    property int defaultSeconds: 3
    property int seconds: defaultSeconds
    height: 200
    width: 200
    opacity: 0
    anchors.centerIn: parent

    Text
    {
        anchors.centerIn: parent
        text: innerTimer.running ? countDown.seconds : qsTr("Smile")
        font.pointSize: 100
        font.family: "DejaVu Serif"
        color: "white"
    }

    /*Image {
        id: countImg
        source: (innerTimer.running && countDown.seconds > 0) ? "img/countdown/"+countDown.seconds+".png" : "img/countdown/go.png"
        anchors.centerIn: parent
    }*/

    Timer {
        id: innerTimer
        repeat: true
        interval: 1000
        onTriggered: {
            countDown.seconds--;
            if (countDown.seconds == 0) {
                running = false;
                countDown.seconds = countDown.defaultSeconds
                countDown.opacity = 0
                countDown.timeout()
            }
        }
    }

    Behavior on opacity {
        PropertyAnimation { duration: 500 }
    }

    function start() {
        seconds = defaultSeconds
        opacity = 0.7;
        innerTimer.start();
    }

    function stop() {
        opacity = 0;
        innerTimer.stop();
    }
}
