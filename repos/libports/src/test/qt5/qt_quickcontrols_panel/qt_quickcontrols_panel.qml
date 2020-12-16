/*
 * QML example from the Qt5 Quick Controls 2
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2
import LocalIO 1.0


ApplicationWindow {
	id:      root
	title:   "Panel"
	visible: true

	background: Rectangle { color: "black" }

	palette.button: "transparent"

	FileReport {
		id:      buttonReport
		target:  "/reports/button_list.xml"
		onError: console.log(msg)
	}

	function report () {
		var buttonList = "<button_list>
	<button label=\"webbrowser\" hold=\"%1\"/>
	<button label=\"screencast\" hold=\"%2\"/>
	<button label=\"keyboard\"   hold=\"%3\"/>
</button_list>"
		buttonReport.write(buttonList.arg(browserButton.checked.toString()).arg(screenButton.checked.toString()).arg(keyboardButton.checked.toString()));
	}

	Row {
		anchors {
			horizontalCenter: parent.horizontalCenter
			top: parent.top
		}
		spacing: 20

		Button {
			id:        screenButton
			onClicked: root.report()
			autoExclusive: true
			checkable: true
			icon.source: "content/video-display-symbolic.svg"
			icon.color: "transparent"
		}

		Button {
			id:        browserButton
			onClicked: root.report()
			autoExclusive: true
			checkable: true
			icon.source: "content/web-browser-symbolic.svg"
			icon.color: "transparent"
		}
	}

	Row {
		anchors {
			right: parent.right
			top: parent.top
		}
		Button {
			id:        keyboardButton
			onClicked: root.report()
			checkable: true
			icon.source: "content/input-keyboard-symbolic.svg"
			icon.color: "transparent"
		}
	}
}

/* vi: set ft=javascript : */
