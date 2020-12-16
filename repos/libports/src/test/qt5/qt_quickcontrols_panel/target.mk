QMAKE_PROJECT_FILE = $(PRG_DIR)/qt_quickcontrols_panel.pro

QMAKE_TARGET_BINARIES = test-qt_quickcontrols_panel

QT5_PORT_LIBS += libQt5Core libQt5Gui libQt5Network
QT5_PORT_LIBS += libQt5Qml libQt5Quick libQt5QuickTemplates2 libQt5QuickControls2

LIBS = libc libm mesa qt5_component stdcxx $(QT5_PORT_LIBS)

include $(call select_from_repositories,lib/import/import-qt5_qmake.mk)
