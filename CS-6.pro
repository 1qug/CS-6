QT += widgets network serialport svg

CONFIG += c++17
TEMPLATE = app
TARGET = CS-6

INCLUDEPATH += $$PWD/src

RESOURCES += resources/resources.qrc

SOURCES += \
    src/main.cpp \
    src/core/siggenclient.cpp \
    src/core/protocol/siggenprotocol.cpp \
    src/core/protocol/usrm0protocol.cpp \
    src/core/usrm0client.cpp \
    src/transport/serialtransport.cpp \
    src/transport/tcptransport.cpp \
    src/ui/mainwindow/mainwindow.cpp \
    src/ui/pages/generator/calibrationpage.cpp \
    src/ui/pages/generator/connectionpage.cpp \
    src/ui/pages/generator/generatorpage.cpp \
    src/ui/pages/generator/homepage.cpp \
    src/ui/pages/generator/modulationpage.cpp \
    src/ui/pages/generator/noisepage.cpp \
    src/ui/pages/generator/referencepage.cpp \
    src/ui/pages/generator/sweeppage.cpp \
    src/ui/pages/generator/systeminfopage.cpp \
    src/ui/pages/lanconfig/lanconfigpage.cpp \
    src/ui/theme.cpp

HEADERS += \
    src/app/appinfo.h \
    src/core/siggenclient.h \
    src/core/protocol/siggenprotocol.h \
    src/core/protocol/usrm0protocol.h \
    src/core/usrm0client.h \
    src/transport/itransport.h \
    src/transport/serialtransport.h \
    src/transport/tcptransport.h \
    src/ui/mainwindow/mainwindow.h \
    src/ui/pages/generator/calibrationpage.h \
    src/ui/pages/generator/connectionpage.h \
    src/ui/pages/generator/generatorpage.h \
    src/ui/pages/generator/generatorpages.h \
    src/ui/pages/generator/homepage.h \
    src/ui/pages/generator/modulationpage.h \
    src/ui/pages/generator/noisepage.h \
    src/ui/pages/generator/referencepage.h \
    src/ui/pages/generator/sweeppage.h \
    src/ui/pages/generator/systeminfopage.h \
    src/ui/pages/lanconfig/lanconfigpage.h \
    src/ui/theme.h

FORMS += \
    src/ui/mainwindow/mainwindow.ui \
    src/ui/pages/generator/calibrationpage.ui \
    src/ui/pages/generator/connectionpage.ui \
    src/ui/pages/generator/generatorpage.ui \
    src/ui/pages/generator/homepage.ui \
    src/ui/pages/generator/modulationpage.ui \
    src/ui/pages/generator/noisepage.ui \
    src/ui/pages/generator/referencepage.ui \
    src/ui/pages/generator/sweeppage.ui \
    src/ui/pages/generator/systeminfopage.ui \
    src/ui/pages/lanconfig/lanconfigpage.ui

# Keep debug and release build output inside build/.
CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/build/debug
    OBJECTS_DIR = $$PWD/build/debug
    MOC_DIR = $$PWD/build/debug
    RCC_DIR = $$PWD/build/debug
    UI_DIR = $$PWD/build/debug
} else {
    DESTDIR = $$PWD/build/release
    OBJECTS_DIR = $$PWD/build/release
    MOC_DIR = $$PWD/build/release
    RCC_DIR = $$PWD/build/release
    UI_DIR = $$PWD/build/release
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
