QT       += core gui
QT       += widgets charts

# Указываем путь к SDK, чтобы найти OpenGL и другие системные фреймворки
QMAKE_MAC_SDK = macosx
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += sdk_no_version_check

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Euler/EulerModel.cpp \
    Euler/EulerWidget.cpp \
    Heun/HeunModel.cpp \
    Heun/HeunWidget.cpp \
    SecondOrderModel.cpp \
    SecondOrderWidget.cpp \
    main.cpp \
    MainWindow.cpp \
    analysis/SwitchingAnalysis.cpp \
    analysis/MSTChartWidget.cpp

HEADERS += \
    Euler/EulerModel.hpp \
    Euler/EulerWidget.hpp \
    Heun/HeunModel.hpp \
    Heun/HeunWidget.hpp \
    MainWindow.hpp \
    SecondOrderModel.hpp \
    SecondOrderWidget.hpp \
    analysis/SwitchingAnalysis.hpp \
    analysis/MSTChartWidget.hpp

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# On modern macOS SDKs the ancient AGL framework may be missing and
# cause the linker to fail (ld: framework 'AGL' not found). Prevent
# qmake from adding AGL to the generated Makefile.

QMAKE_LFLAGS += -Wl,-framework,OpenGL
LIBS -= -framework AGL

# Be extra defensive: unset AGL from any qmake variables that might include it
macx {
    QMAKE_LIBS = $$replace(QMAKE_LIBS, -framework AGL, )
    QMAKE_LFLAGS_RELEASE = $$replace(QMAKE_LFLAGS_RELEASE, -framework AGL, )
    QMAKE_LFLAGS_DEBUG = $$replace(QMAKE_LFLAGS_DEBUG, -framework AGL, )
}
