INCLUDEPATH += $$PWD
SOURCES += $$PWD/openglwindow.cpp
HEADERS += $$PWD/openglwindow.h

SOURCES += \
    main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/gui/openglwindow
INSTALLS += target
QMAKE_MAC_SDK = macosx10.11

RESOURCES += \
    gestionnaire.qrc
