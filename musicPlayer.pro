QT += widgets

# 基础模块
QT += core gui

# 关键：多媒体模块（播放音频必须）
QT += multimedia multimediawidgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    songlibrarywindow.cpp

HEADERS += \
    mainwindow.h \
    songlibrarywindow.h

FORMS += \
    mainwindow.ui

# Windows优化：隐藏控制台窗口
win32: CONFIG -= console
win32: CONFIG -= app_bundle


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
