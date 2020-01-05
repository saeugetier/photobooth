import QtQuick 2.4
import CollageModel 1.0
import QtQuick.Controls 2.5

ApplicationFlowForm {
    id: form
    width: parent.width
    height: parent.height

    mainMenuModel: modelFactory.getCollageIconModel()
}


