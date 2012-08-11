SOURCES += $$PWD/src/libenjoy.c
HEADERS += $$PWD/src/libenjoy.h \
    $$PWD/src/libenjoy_p.h

INCLUDEPATH += $$PWD/src

win32 {
    SOURCES += $$PWD/src/libenjoy_win32.c
    HEADERS += $$PWD/src/libenjoy_win32.h
}

linux-* {
    SOURCES += $$PWD/src/libenjoy_linux.c
    HEADERS += $$PWD/src/libenjoy_linux.h
}
