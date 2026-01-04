QT += core gui widgets opengl

CONFIG += c++17

SOURCES += \
    battledialog.cpp \
    battlefielddialog.cpp \
    battlesetupdialog.cpp \
    eventdialog.cpp \
    gamecontroller.cpp \
    graphicsframe.cpp \
    graphicsview.cpp \
    main.cpp \
    mainwindow.cpp \
    mapgraph.cpp \
    navprogress.cpp \
    pieceentrywidget.cpp \
    pieceitem.cpp \
    piecelistwidget.cpp \
    placementmanager.cpp \
    regionitem.cpp \
    strikegroupdialog.cpp \
    util.cpp

HEADERS += \
    battlecontext.h \
    battledialog.h \
    battlefielddialog.h \
    battlesetupdialog.h \
    dragdrop.h \
    eventdialog.h \
    gamecontroller.h \
    graphicsframe.h \
    graphicsview.h \
    mainwindow.h \
    mapgraph.h \
    navprogress.h \
    pieceentrywidget.h \
    pieceitem.h \
    piecelistwidget.h \
    placementmanager.h \
    regionitem.h \
    strikegroupdialog.h \
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
