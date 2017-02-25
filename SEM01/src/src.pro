QT += \
	core \
	gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SEM01
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG(debug, debug|release) {
	DESTDIR = ../debug/bin/
	OBJECTS_DIR = ../debug/.obj/
	LIBS += -L../debug/lib/
}

CONFIG(release, debug|release) {
	DESTDIR = ../release/bin/
	OBJECTS_DIR = ../release/.obj/
	LIBS += -L../release/lib/
}

INCLUDEPATH += ../include/

SOURCES += \
	main.cpp\
	MainWindow.cpp

HEADERS += \
	MainWindow.hpp

FORMS += \
	MainWindow.ui
