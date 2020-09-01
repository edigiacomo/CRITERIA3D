#--------------------------------------------------------
#
#   shapeUtilities
#   This project is part of CRITERIA-3D distribution
#
#--------------------------------------------------------

QT    -= gui
QT    += core widgets sql

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/shapeUtilities
    } else {
        TARGET = release/shapeUtilities
    }
}
win32:{
    TARGET = shapeUtilities
}

TEMPLATE = lib
CONFIG += staticlib

DEFINES += _CRT_SECURE_NO_WARNINGS

INCLUDEPATH =  ../crit3dDate ../mathFunctions ../gis ../shapeHandler  ../utilities ../project

SOURCES += \
    ../project/formInfo.cpp \
    shapeFromCsv.cpp \
    shapeToRaster.cpp    \
    shapeUtilities.cpp   \
    unitCropMap.cpp      \
    zonalStatistic.cpp


HEADERS += \
    ../project/formInfo.h \
    shapeFromCsv.h \
    shapeToRaster.h    \
    shapeUtilities.h   \
    unitCropMap.h      \
    zonalStatistic.h

