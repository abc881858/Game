QT += core gui widgets opengl

CONFIG += c++17

SOURCES += \
    eventdialog.cpp \
    graphicsframe.cpp \
    graphicsview.cpp \
    main.cpp \
    mainwindow.cpp \
    pieceentrywidget.cpp \
    pieceitem.cpp \
    piecelistwidget.cpp \
    util.cpp

HEADERS += \
    cityslotitem.h \
    eventdialog.h \
    graphicsframe.h \
    graphicsview.h \
    mainwindow.h \
    pieceentrywidget.h \
    pieceitem.h \
    piecelistwidget.h \
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
