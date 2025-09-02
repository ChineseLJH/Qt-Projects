QT += quick virtualkeyboard network
CONFIG += c++17

SOURCES += \
        main.cpp \
        myudp.cpp \
        tcpclient.cpp

HEADERS += \
    myudp.h \
    tcpclient.h

resources.prefix = /$${TARGET}
RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
