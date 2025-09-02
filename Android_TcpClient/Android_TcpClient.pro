QT += quick virtualkeyboard network
CONFIG += c++17

# 只生成 arm64-v8a 架构的 APK
# android {
#     ANDROID_ABIS = arm64-v8a
# }

SOURCES += \
    main.cpp \
    TcpClient.cpp

HEADERS += \
    TcpClient.h

RESOURCES += qml.qrc


qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# RESOURCES += \
#     qml.qrc

