QT       += core gui widgets sql charts printsupport

CONFIG   += c++17
TARGET    = Devena
TEMPLATE  = app

SOURCES += \
    main.cpp \
    connection.cpp \
    basedemo.cpp \
    evaluation.cpp \
    evaluationmodel.cpp \
    mainwindow.cpp

HEADERS += \
    connection.h \
    basedemo.h \
    evaluation.h \
    evaluationmodel.h \
    mainwindow.h \
    chartview.h \
    theme.h

FORMS += \
    mainwindow.ui

DISTFILES += \
    database/devena.sql \
    README.md
