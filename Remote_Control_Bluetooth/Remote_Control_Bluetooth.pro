QT += core gui bluetooth network qml quick quickcontrols2

CONFIG += c++17 qtquickcompiler

SOURCES += \
    main.cpp \
    bluetoothmanager.cpp

HEADERS += \
    bluetoothmanager.h

RESOURCES += \
    qml.qrc

# 启用 Qt for Android 打包插件
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

# QML 插件搜索路径（如果你使用了自定义模块）
QML_IMPORT_PATH += $$PWD

# 图标（可选）
# ICON = logo.ico

# Android 权限设置
DISTFILES += \
    android/AndroidManifest.xml \
    android/res/xml/qtprovider_paths.xml

