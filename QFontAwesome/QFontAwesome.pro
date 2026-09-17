TARGET = QFontAwesome
TEMPLATE = lib
CONFIG += staticlib c++17
QT += widgets

# generated headers: pass QFONTAWESOME_GENERATED_DIR=... to qmake, or keep a local (git-ignored) copy here
isEmpty(QFONTAWESOME_GENERATED_DIR): QFONTAWESOME_GENERATED_DIR = $$PWD/fonts/generated
include(QFontAwesome.pri)

isEmpty(PREFIX) {
    unix {
        PREFIX = /usr
    } else {
        PREFIX = $$[QT_INSTALL_PREFIX]
    }
}

install_headers.files = QFontAwesome.h QFontAwesomeAnim.h
install_headers.path = $$PREFIX/include
target.path = $$PREFIX/lib
INSTALLS += install_headers target
