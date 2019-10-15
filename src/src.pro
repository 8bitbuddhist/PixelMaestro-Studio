#-------------------------------------------------
#
# Project created by QtCreator 2017-05-16T00:28:49
#
#-------------------------------------------------

QT       += core gui widgets serialport

TARGET = PixelMaestro_Studio
TEMPLATE = app
DEFINES += BUILD_VERSION=\\\"v0.50.2\\\" PIXEL_ENABLE_ACCURATE_FADING CANVAS_ENABLE_FONTS
android {
QMAKE_LINK += -nostdlib++
QMAKE_LFLAGS += -nostdlib++
}
!android {
QMAKE_CXXFLAGS = -std=c++11 -Wall -Wno-unused-parameter -Wno-reorder -Wno-switch
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
}


SOURCES += main.cpp\
drawingarea/maestrodrawingarea.cpp \
controller/maestrocontroller.cpp \
../lib/PixelMaestro/src/canvas/fonts/font5x8.cpp \
window/mainwindow.cpp \
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
controller/palettecontroller.cpp \
controller/showcontroller.cpp \
../lib/PixelMaestro/src/colorpresets.cpp \
../lib/PixelMaestro/src/cue/cuehandler.cpp \
utility/canvasutility.cpp \
../lib/PixelMaestro/src/cue/showcuehandler.cpp \
widget/maestrocontrolwidget.cpp \
widget/palettecontrolwidget.cpp \
widget/animation/lightninganimationcontrolwidget.cpp \
widget/animation/plasmaanimationcontrolwidget.cpp \
widget/animation/radialanimationcontrolwidget.cpp \
widget/animation/sparkleanimationcontrolwidget.cpp \
dialog/preferencesdialog.cpp \
../lib/PixelMaestro/src/animation/fireanimation.cpp \
widget/animation/fireanimationcontrolwidget.cpp \
../lib/PixelMaestro/src/core/timer/timer.cpp \
../lib/PixelMaestro/src/core/timer/animationtimer.cpp \
widget/animation/waveanimationcontrolwidget.cpp \
drawingarea/sectiondrawingarea.cpp \
dialog/maestrodrawingareadialog.cpp \
../lib/PixelMaestro/src/core/palette.cpp \
widget/devicecontrolwidget.cpp \
dialog/cueinterpreterdialog.cpp \
dialog/paletteeditdialog.cpp \
utility/cueinterpreter.cpp \
widget/animationcontrolwidget.cpp \
widget/showcontrolwidget.cpp \
widget/sectioncontrolwidget.cpp \
widget/canvascontrolwidget.cpp \
dialog/sectionmapdialog.cpp \
controller/serialdevicecontroller.cpp \
controller/serialdevicethreadcontroller.cpp \
model/sectionmapmodel.cpp \
dialog/editeventdialog.cpp \
model/cuemodel.cpp \
dialog/adddevicedialog.cpp

HEADERS += \
drawingarea/maestrodrawingarea.h \
controller/maestrocontroller.h \
../lib/PixelMaestro/src/canvas/fonts/font.h \
../lib/PixelMaestro/src/canvas/fonts/font5x8.h \
window/mainwindow.h \
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
controller/palettecontroller.h \
controller/showcontroller.h \
../lib/PixelMaestro/src/colorpresets.h \
utility/canvasutility.h \
../lib/PixelMaestro/src/cue/showcuehandler.h \
widget/maestrocontrolwidget.h \
widget/palettecontrolwidget.h \
widget/animation/lightninganimationcontrolwidget.h \
widget/animation/plasmaanimationcontrolwidget.h \
widget/animation/radialanimationcontrolwidget.h \
widget/animation/sparkleanimationcontrolwidget.h \
dialog/preferencesdialog.h \
../lib/PixelMaestro/src/animation/fireanimation.h \
widget/animation/fireanimationcontrolwidget.h \
../lib/PixelMaestro/src/core/timer/timer.h \
../lib/PixelMaestro/src/core/timer/animationtimer.h \
widget/animation/waveanimationcontrolwidget.h \
drawingarea/sectiondrawingarea.h \
dialog/maestrodrawingareadialog.h \
../lib/PixelMaestro/src/core/palette.h \
widget/devicecontrolwidget.h \
dialog/cueinterpreterdialog.h \
dialog/paletteeditdialog.h \
utility/cueinterpreter.h \
widget/animationcontrolwidget.h \
widget/showcontrolwidget.h \
widget/sectioncontrolwidget.h \
widget/canvascontrolwidget.h \
dialog/sectionmapdialog.h \
controller/serialdevicecontroller.h \
controller/serialdevicethreadcontroller.h \
model/sectionmapmodel.h \
dialog/editeventdialog.h \
model/cuemodel.h \
dialog/adddevicedialog.h

FORMS	+= window/mainwindow.ui \
widget/maestrocontrolwidget.ui \
widget/palettecontrolwidget.ui \
widget/animation/lightninganimationcontrolwidget.ui \
widget/animation/plasmaanimationcontrolwidget.ui \
widget/animation/radialanimationcontrolwidget.ui \
widget/animation/sparkleanimationcontrolwidget.ui \
dialog/preferencesdialog.ui \
widget/animation/fireanimationcontrolwidget.ui \
widget/animation/waveanimationcontrolwidget.ui \
dialog/maestrodrawingareadialog.ui \
widget/devicecontrolwidget.ui \
dialog/cueinterpreterdialog.ui \
dialog/paletteeditdialog.ui \
widget/animationcontrolwidget.ui \
widget/showcontrolwidget.ui \
widget/sectioncontrolwidget.ui \
widget/canvascontrolwidget.ui \
dialog/sectionmapdialog.ui \
dialog/editeventdialog.ui \
dialog/adddevicedialog.ui

INCLUDEPATH += \
$$PWD/src \
$$PWD/../lib/PixelMaestro/src

RESOURCES += resources/images/images.qrc \
resources/fonts/fonts.qrc
