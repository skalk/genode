TEMPLATE = app
TARGET = virtualkeyboard
QT += qml quick quickwidgets
SOURCES += main.cpp

RESOURCES += \
    virtualkeyboard.qrc

OTHER_FILES += \
    virtualkeyboard.qml \
    content/AutoScroller.qml \
    content/HandwritingModeButton.qml \
    content/TextArea.qml \
    content/TextField.qml \

