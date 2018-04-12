#-------------------------------------------------
#
# Project created by QtCreator 2018-03-08T15:53:41
#
#-------------------------------------------------

# QT       += testlib

QT       -= gui

TARGET	  = tests/tests/colorstest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
		../lib/PixelMaestro/src/canvas/fonts/font5x8.cpp \
		../lib/PixelMaestro/src/animation/blinkanimation.cpp \
		../lib/PixelMaestro/src/animation/cycleanimation.cpp \
		../lib/PixelMaestro/src/animation/randomanimation.cpp \
		../lib/PixelMaestro/src/animation/solidanimation.cpp \
		../lib/PixelMaestro/src/animation/sparkleanimation.cpp \
		../lib/PixelMaestro/src/animation/waveanimation.cpp \
		../lib/PixelMaestro/src/animation/animation.cpp \
		../lib/PixelMaestro/src/core/colors.cpp \
		../lib/PixelMaestro/src/core/maestro.cpp \
		../lib/PixelMaestro/src/core/pixel.cpp \
		../lib/PixelMaestro/src/core/point.cpp \
		../lib/PixelMaestro/src/core/section.cpp \
		../lib/PixelMaestro/src/utility.cpp \
		../lib/PixelMaestro/src/animation/radialanimation.cpp \
		../lib/PixelMaestro/src/animation/mandelbrotanimation.cpp \
		../lib/PixelMaestro/src/animation/plasmaanimation.cpp \
		../lib/PixelMaestro/src/animation/lightninganimation.cpp \
		../lib/PixelMaestro/src/canvas/canvas.cpp \
		../lib/PixelMaestro/src/cue/event.cpp \
		../lib/PixelMaestro/src/cue/show.cpp \
		../lib/PixelMaestro/src/cue/cuecontroller.cpp \
		../lib/PixelMaestro/src/cue/animationcuehandler.cpp \
		../lib/PixelMaestro/src/cue/canvascuehandler.cpp \
		../lib/PixelMaestro/src/cue/maestrocuehandler.cpp \
		../lib/PixelMaestro/src/cue/sectioncuehandler.cpp \
		../lib/PixelMaestro/src/colorpresets.cpp \
		../lib/PixelMaestro/src/cue/cuehandler.cpp \
		../lib/PixelMaestro/src/cue/showcuehandler.cpp \
		../lib/PixelMaestro/src/animation/fireanimation.cpp \
		../lib/PixelMaestro/src/core/timer/timer.cpp \
		../lib/PixelMaestro/src/core/timer/animationtimer.cpp \
		../lib/PixelMaestro/src/animation/mappedanimation.cpp \
		../lib/PixelMaestro/src/core/palette.cpp \
	../lib/PixelMaestro/tests/tests/colorstest.cpp \
	../lib/PixelMaestro/tests/tests/maestrotest.cpp \
	../lib/PixelMaestro/tests/tests/pixeltest.cpp \
	../lib/PixelMaestro/tests/tests/sectiontest.cpp \
	../lib/PixelMaestro/tests/tests/showtest.cpp \
	../lib/PixelMaestro/tests/tests/utilitytest.cpp \
	../lib/PixelMaestro/tests/tests/timertest.cpp \
	../lib/PixelMaestro/tests/tests/animationtimertest.cpp

HEADERS += \
		.../lib/PixelMaestro/src/canvas/fonts/font.h \
		../lib/PixelMaestro/src/canvas/fonts/font5x8.h \
		../lib/PixelMaestro/src/animation/blinkanimation.h \
		../lib/PixelMaestro/src/animation/cycleanimation.h \
		../lib/PixelMaestro/src/animation/randomanimation.h \
		../lib/PixelMaestro/src/animation/solidanimation.h \
		../lib/PixelMaestro/src/animation/sparkleanimation.h \
		../lib/PixelMaestro/src/animation/waveanimation.h \
		../lib/PixelMaestro/src/animation/animation.h \
		../lib/PixelMaestro/src/animation/animationtype.h \
		../lib/PixelMaestro/src/core/colors.h \
		../lib/PixelMaestro/src/core/maestro.h \
		../lib/PixelMaestro/src/core/pixel.h \
		../lib/PixelMaestro/src/core/point.h \
		../lib/PixelMaestro/src/core/section.h \
		../lib/PixelMaestro/src/utility.h \
		../lib/PixelMaestro/src/animation/radialanimation.h \
		../lib/PixelMaestro/src/animation/mandelbrotanimation.h \
		../lib/PixelMaestro/src/animation/plasmaanimation.h \
		../lib/PixelMaestro/src/animation/lightninganimation.h \
		../lib/PixelMaestro/src/canvas/canvas.h \
		../lib/PixelMaestro/src/cue/event.h \
		../lib/PixelMaestro/src/cue/show.h \
		../lib/PixelMaestro/src/cue/cuecontroller.h \
		../lib/PixelMaestro/src/cue/cuehandler.h \
		../lib/PixelMaestro/src/cue/animationcuehandler.h \
		../lib/PixelMaestro/src/cue/canvascuehandler.h \
		../lib/PixelMaestro/src/cue/maestrocuehandler.h \
		../lib/PixelMaestro/src/cue/sectioncuehandler.h \
		../lib/PixelMaestro/src/colorpresets.h \
		../lib/PixelMaestro/src/cue/showcuehandler.h \
		../lib/PixelMaestro/src/animation/fireanimation.h \
		../lib/PixelMaestro/src/core/timer/timer.h \
		../lib/PixelMaestro/src/core/timer/animationtimer.h \
		../lib/PixelMaestro/src/animation/mappedanimation.h \
		../lib/PixelMaestro/src/core/palette.h \
	../lib/PixelMaestro/tests/tests/colorstest.h \
	../lib/PixelMaestro/tests/tests/maestrotest.h \
	../lib/PixelMaestro/tests/tests/pixeltest.h \
	../lib/PixelMaestro/tests/tests/sectiontest.h \
	../lib/PixelMaestro/tests/tests/showtest.h \
	../lib/PixelMaestro/tests/tests/utilitytest.h \
	../lib/PixelMaestro/tests/catch/include/catch.hpp

INCLUDEPATH += \
		$$PWD/../lib/PixelMaestro/src \
		$$PWD/../lib/PixelMaestro/tests/tests \
		$$PWD/../lib/PixelMaestro/tests/catch/include

DEFINES += SRCDIR=\\\"$$PWD/\\\"
