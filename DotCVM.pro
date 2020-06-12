CONFIG -= qt

TEMPLATE = lib
DEFINES += DOTCVM_LIBRARY

CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    devices.cpp \
    file_utils.cpp \
    jinterface/app_main.cpp \
    main.cpp

HEADERS += \
    app_main.h \
    devices.h \
    file_utils.h \
    j_app_main.h \
    log.h \
    main.h

INCLUDEPATH += $$(JAVA_HOME)/include
INCLUDEPATH += $$(JAVA_HOME)/include/linux

unix
{
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    AppMain.java \
    build/jclean.sh \
    build/jcompile.sh
