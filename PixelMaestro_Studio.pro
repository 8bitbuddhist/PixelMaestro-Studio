#-------------------------------------------------
#
# Project created by QtCreator 2017-05-16T00:28:49
#
#-------------------------------------------------

QT       += core gui widgets serialport

TARGET = PixelMaestro_Studio
TEMPLATE = app
QMAKE_CXXFLAGS = -std=c++11 -Wunused -Wno-unused-parameter
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

SOURCES += src/main.cpp\
		src/drawingarea/maestrodrawingarea.cpp \
		src/drawingarea/simpledrawingarea.cpp \
		src/demo/blinkdemo.cpp \
		src/widget/maestrocontrol.cpp \
		src/controller/maestrocontroller.cpp \
		src/demo/showdemo.cpp \
		lib/PixelMaestro/src/canvas/fonts/font5x8.cpp \
		src/demo/canvasdemo.cpp \
		src/drawingarea/canvasdrawingarea.cpp \
		src/window/mainwindow.cpp \
		lib/PixelMaestro/src/animation/blinkanimation.cpp \
		lib/PixelMaestro/src/animation/cycleanimation.cpp \
		lib/PixelMaestro/src/animation/mergeanimation.cpp \
		lib/PixelMaestro/src/animation/randomanimation.cpp \
		lib/PixelMaestro/src/animation/solidanimation.cpp \
		lib/PixelMaestro/src/animation/sparkleanimation.cpp \
		lib/PixelMaestro/src/animation/waveanimation.cpp \
		lib/PixelMaestro/src/animation/animation.cpp \
		lib/PixelMaestro/src/core/colors.cpp \
		lib/PixelMaestro/src/core/maestro.cpp \
		lib/PixelMaestro/src/core/pixel.cpp \
		lib/PixelMaestro/src/core/point.cpp \
		lib/PixelMaestro/src/core/section.cpp \
		lib/PixelMaestro/src/utility.cpp \
		lib/PixelMaestro/src/animation/radialanimation.cpp \
		lib/PixelMaestro/src/animation/mandelbrotanimation.cpp \
		lib/PixelMaestro/src/animation/plasmaanimation.cpp \
		lib/PixelMaestro/src/animation/lightninganimation.cpp \
		src/widget/animation/lightninganimationcontrol.cpp \
		src/widget/animation/plasmaanimationcontrol.cpp \
		src/widget/animation/sparkleanimationcontrol.cpp \
		lib/PixelMaestro/src/canvas/colorcanvas.cpp \
		src/demo/colorcanvasdemo.cpp \
		lib/PixelMaestro/src/canvas/animationcanvas.cpp \
		lib/PixelMaestro/src/canvas/canvas.cpp \
		src/demo/cuedemo.cpp \
		lib/PixelMaestro/src/cue/event.cpp \
		lib/PixelMaestro/src/cue/show.cpp \
		lib/PixelMaestro/src/cue/cuecontroller.cpp \
		lib/PixelMaestro/src/cue/animationcuehandler.cpp \
		lib/PixelMaestro/src/cue/canvascuehandler.cpp \
		lib/PixelMaestro/src/cue/maestrocuehandler.cpp \
		lib/PixelMaestro/src/cue/sectioncuehandler.cpp \
		src/window/settingsdialog.cpp \
		src/widget/animation/radialanimationcontrol.cpp \
		src/controller/palettecontroller.cpp \
		src/widget/palettecontrol.cpp \
		src/controller/cueinterpreter.cpp \
		src/controller/showcontroller.cpp \
		src/demo/animatedcanvasdemo.cpp \
		lib/PixelMaestro/src/colorpresets.cpp \
		lib/PixelMaestro/src/cue/cuehandler.cpp \
		src/utility/canvasutility.cpp \
		src/window/virtualserialdevicedialog.cpp \
		lib/PixelMaestro/src/cue/showcuehandler.cpp \
    lib/PixelMaestro/src/core/timing/timing.cpp \
    lib/PixelMaestro/src/core/timing/animationtiming.cpp

HEADERS += \
		src/demo/blinkdemo.h \
		src/drawingarea/maestrodrawingarea.h \
		src/drawingarea/simpledrawingarea.h \
		src/widget/maestrocontrol.h \
		src/controller/maestrocontroller.h \
		src/demo/showdemo.h \
		lib/PixelMaestro/src/canvas/fonts/font.h \
		lib/PixelMaestro/src/canvas/fonts/font5x8.h \
		src/demo/canvasdemo.h \
		src/drawingarea/canvasdrawingarea.h \
		src/window/mainwindow.h \
		lib/PixelMaestro/src/animation/blinkanimation.h \
		lib/PixelMaestro/src/animation/cycleanimation.h \
		lib/PixelMaestro/src/animation/mergeanimation.h \
		lib/PixelMaestro/src/animation/randomanimation.h \
		lib/PixelMaestro/src/animation/solidanimation.h \
		lib/PixelMaestro/src/animation/sparkleanimation.h \
		lib/PixelMaestro/src/animation/waveanimation.h \
		lib/PixelMaestro/src/animation/animation.h \
		lib/PixelMaestro/src/core/colors.h \
		lib/PixelMaestro/src/core/maestro.h \
		lib/PixelMaestro/src/core/pixel.h \
		lib/PixelMaestro/src/core/point.h \
		lib/PixelMaestro/src/core/section.h \
		lib/PixelMaestro/src/utility.h \
		lib/PixelMaestro/src/animation/radialanimation.h \
		lib/PixelMaestro/src/animation/mandelbrotanimation.h \
		lib/PixelMaestro/src/animation/plasmaanimation.h \
		lib/PixelMaestro/src/animation/lightninganimation.h \
		src/widget/animation/lightninganimationcontrol.h \
		src/widget/animation/plasmaanimationcontrol.h \
		src/widget/animation/sparkleanimationcontrol.h \
		lib/PixelMaestro/src/canvas/colorcanvas.h \
		src/demo/colorcanvasdemo.h \
		lib/PixelMaestro/src/canvas/canvastype.h \
		lib/PixelMaestro/src/canvas/animationcanvas.h \
		lib/PixelMaestro/src/canvas/canvas.h \
		src/demo/cuedemo.h \
		lib/PixelMaestro/src/cue/event.h \
		lib/PixelMaestro/src/cue/show.h \
		lib/PixelMaestro/src/cue/cuecontroller.h \
		lib/PixelMaestro/src/cue/cuehandler.h \
		lib/PixelMaestro/src/cue/animationcuehandler.h \
		lib/PixelMaestro/src/cue/canvascuehandler.h \
		lib/PixelMaestro/src/cue/maestrocuehandler.h \
		lib/PixelMaestro/src/cue/sectioncuehandler.h \
		lib/PixelMaestro/src/animation/animationtype.h \
		src/window/settingsdialog.h \
		src/widget/animation/radialanimationcontrol.h \
		src/controller/palettecontroller.h \
		src/widget/palettecontrol.h \
		src/controller/cueinterpreter.h \
		src/controller/showcontroller.h \
		src/demo/animatedcanvasdemo.h \
		lib/PixelMaestro/src/colorpresets.h \
		src/utility/canvasutility.h \
		src/window/virtualserialdevicedialog.h \
		lib/PixelMaestro/src/cue/showcuehandler.h \
    lib/PixelMaestro/src/core/timing/timing.h \
    lib/PixelMaestro/src/core/timing/animationtiming.h

FORMS	+= \
		src/widget/maestrocontrol.ui \
		src/window/mainwindow.ui \
		src/widget/animation/lightninganimationcontrol.ui \
		src/widget/animation/plasmaanimationcontrol.ui \
		src/widget/animation/sparkleanimationcontrol.ui \
		src/window/settingsdialog.ui \
		src/widget/animation/radialanimationcontrol.ui \
		src/widget/palettecontrol.ui \
		src/window/virtualserialdevicedialog.ui

INCLUDEPATH += \
		$$PWD/src \
		$$PWD/lib/PixelMaestro/src

DISTFILES += \
		src/resources/nyan.gif \
		src/resources/nyan.gif

RESOURCES += \
		src/resources.qrc
