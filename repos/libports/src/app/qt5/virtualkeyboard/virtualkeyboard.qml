import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Window 2.2
import QtQuick.VirtualKeyboard 2.2
import QtQuick.VirtualKeyboard.Settings 2.2
import "content"
import Im_enabled_item 1.0

Im_enabled_item {

	// this is important to enable the shift key
	focus: true

/*
	TextField {
		width: parent.width
		height: parent.height
		inputMethodHints: Qt.ImhPreferLowercase
		//focus: true
	}
*/

	Rectangle {
		width: parent.width
		height: parent.height
		color: "black"
	}

    InputPanel {
        id: inputPanel
        y: parent.height - inputPanel.height
        anchors.left: parent.left
        anchors.right: parent.right
        AutoScroller {}
    }

}
