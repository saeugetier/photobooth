import QtQuick 2.4

TimeSettingsForm {
    id: form

    signal setTime(date time)

    property date currentTime : new Date()

    onOpened:
    {
        currentTime = new Date()
        updateForm(currentTime)
    }

    buttonCancel.onClicked:
    {
        close()
    }

    buttonApply.onClicked:
    {
        setTime(currentTime)
        close()
    }

    Timer
    {
        interval: 1000
        running: form.opened
        repeat: true
        onTriggered:
        {
            currentTime = new Date(currentTime.getTime() + 1000)
            updateForm(currentTime)
        }
    }

    function updateForm(/*date*/currDate)
    {
        tumblerHour.currentIndex = currDate.getHours()
        tumblerMinute.currentIndex = currDate.getMinutes()
        tumblerSecond.currentIndex = currDate.getSeconds()
    }

    tumblerSecond.onCurrentIndexChanged:
    {
        let time = currentTime
        time.setSeconds(tumblerSecond.currentIndex)
        currentTime = new Date(time.getTime())
    }

    tumblerMinute.onCurrentIndexChanged:
    {
        let time = currentTime
        time.setMinutes(tumblerMinute.currentIndex)
        currentTime = new Date(time.getTime())
    }

    tumblerHour.onCurrentIndexChanged:
    {
        let time = currentTime
        time.setHours(tumblerHour.currentIndex)
        currentTime = new Date(time.getTime())
    }

    /*calendar.onSelectedDateChanged:
    {
        let newDate = calendar.selectedDate
        newDate.setSeconds(tumblerSecond.currentIndex)
        newDate.setMinutes(tumblerMinute.currentIndex)
        newDate.setHours(tumblerHour.currentIndex)
        currentTime = new Date(newDate)
    }*/
}
