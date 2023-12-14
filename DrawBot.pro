QT += core gui widgets serialport network pdf

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    drawingmode.cpp \
    graphicsitems.cpp \
    main.cpp \
    mainwidget.cpp \
    paintwidget.cpp \
    svgconvertor.cpp \
    widgets/portbox.cpp \
    widgets/sectionwidget.cpp \
    widgets/stackedwidget.cpp \
    widgets/volume.cpp \
    widgets/window.cpp

HEADERS += \
    drawingmode.h \
    graphicsitems.h \
    mainwidget.h \
    paintwidget.h \
    svgconvertor.h \
    widgets/portbox.h \
    widgets/sectionwidget.h \
    widgets/stackedwidget.h \
    widgets/volume.h \
    widgets/window.h

FORMS += \
    drawingmode.ui \
    mainwidget.ui \
    svgconvertor.ui \
    widgets/window.ui

win32: {
    RC_FILE = resources.rc
    INCLUDEPATH += opencv\include
    LIBS += -lopengl32
    LIBS += D:\Development\DrawBot\opencv\bin\libopencv_core470.dll
    LIBS += D:\Development\DrawBot\opencv\bin\libopencv_highgui470.dll
    LIBS += D:\Development\DrawBot\opencv\bin\libopencv_imgcodecs470.dll
    LIBS += D:\Development\DrawBot\opencv\bin\libopencv_imgproc470.dll
}
unix: {
    INCLUDEPATH += /usr/local/opencv/include/opencv4
    LIBS += -L"/usr/local/opencv/lib/"
    LIBS += -lopencv_core
    LIBS += -lopencv_highgui
    LIBS += -lopencv_imgcodecs
    LIBS += -lopencv_imgproc
}

RESOURCES += \
    resources.qrc
