QT += core gui widgets opengl

CONFIG += c++17

SOURCES += \
    eventdialog.cpp \
    gamecontroller.cpp \
    graphicsframe.cpp \
    graphicsview.cpp \
    main.cpp \
    mainwindow.cpp \
    navprogress.cpp \
    pieceentrywidget.cpp \
    pieceitem.cpp \
    piecelistwidget.cpp \
    placementmanager.cpp \
    regionitem.cpp \
    util.cpp

HEADERS += \
    dragdrop.h \
    eventdialog.h \
    gamecontroller.h \
    graphicsframe.h \
    graphicsview.h \
    mainwindow.h \
    navprogress.h \
    pieceentrywidget.h \
    pieceitem.h \
    piecelistwidget.h \
    placementmanager.h \
    regionitem.h \
    util.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    res.qrc \
    res/D/res_D.qrc \
    res/S/res_S.qrc \
    res/L/res_L.qrc

INCLUDEPATH += $$PWD/include

LIBS += $$PWD/lib/libqtadvanceddocking.a
