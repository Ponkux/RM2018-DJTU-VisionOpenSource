TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    src/AddBullet/BulletVision.cpp \
    src/ArmorFind/armorfind.cpp \
    src/ArmorFind/armorpredict.cpp \
    src/Camera/RMVideoCapture.cpp \
    src/Camera/StereoXML.cpp \
    src/Serial/CRC_Check.cpp \
    src/Serial/serial.cpp \
    src/Sudoku/digitver/caffe_prototype.cpp \
    src/Sudoku/digitver/HandWritingSolver.cpp \
    src/Sudoku/digitver/ImageProcess.cpp \
    src/Sudoku/digitver/LedNumberSolver.cpp \
    src/Sudoku/digitver/LogicProcess.cpp \
    src/Sudoku/InitParam.cpp \
    src/Sudoku/TString.cpp \
    sudokuposcal.cpp \
    src/WatchDog.cpp

INCLUDEPATH += /usr/local/include \
               /home/ubuntu/caffe-master/include \
               /home/ubuntu/caffe-master/include/caffe \
               /usr/local/cuda/include \
               /usr/local/cuda-8.0/include \
               /usr/local/include/caffe \
               /home/ubuntu/caffe-master/build/src/ \
               /usr/local/include/caffe/proto \



LIBS += /usr/local/lib/libopencv_* \
        /home/ubuntu/caffe-master/build/lib/libcaffe.so \
        /home/ubuntu/caffe-master/build/lib/libcaffe.so.1.0.0 \
        /usr/local/cuda/lib64/lib* \
        /usr/local/cuda-8.0/lib64/lib* \
        -lglog -lboost_system -lpthread

HEADERS += \
    include/AddBullet/BulletVision.h \
    include/ArmorFind/armorfind.h \
    include/ArmorFind/armorpredict.h \
    include/Camera/RMVideoCapture.hpp \
    include/Camera/StereoXML.h \
    include/Serial/CRC_Check.h \
    include/Serial/serial.h \
    include/Sudoku/digitver/caffe_prototype.h \
    include/Sudoku/digitver/HandWritingSolver.h \
    include/Sudoku/digitver/ImageProcess.h \
    include/Sudoku/digitver/LedNumberSolver.h \
    include/Sudoku/digitver/LogicProcess.h \
    include/Sudoku/InitParam.h \
    include/Sudoku/TString.h \
    include/Header.h \
    sudokuposcal.h \
    include/WatchDog.h
