TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    RMVideoCapture.cpp \
    StereoCalib.cpp

INCLUDEPATH += /usr/local/include

LIBS += /usr/local/lib/libopencv_*

HEADERS += \
    RMVideoCapture.hpp \
    StereoCalib.h \
    Header.h

