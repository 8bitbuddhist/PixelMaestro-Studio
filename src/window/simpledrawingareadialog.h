/*
 * VirtualSerialDeviceDialog - Manages a separate Maestro for testing serial output/Cue commands.
 */

#ifndef VIRTUALSERIALDEVICEDIALOG_H
#define VIRTUALSERIALDEVICEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <memory>
#include "controller/maestrocontroller.h"
#include "controller/cueinterpreter.h"
#include "drawingarea/simpledrawingarea.h"

using namespace PixelMaestro;

namespace Ui {
	class SimpleDrawingaAreaDialog;
}

class SimpleDrawingAreaDialog : public QDialog {
		Q_OBJECT

	public:
		QLabel* interpreted = nullptr;

		explicit SimpleDrawingAreaDialog(QWidget *parent, MaestroController* maestro_controller);
		~SimpleDrawingAreaDialog();

	private:
		Ui::SimpleDrawingaAreaDialog *ui;
		MaestroController* maestro_controller_;
		std::unique_ptr<SimpleDrawingArea> drawing_area_;
};

#endif // VIRTUALSERIALDEVICEDIALOG_H
