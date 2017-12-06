/*
 * VirtualSerialDeviceDialog - Manages a separate Maestro for testing serial output/Cue commands.
 */

#ifndef SIMPLEDRAWINGAREADIALOG_H
#define SIMPLEDRAWINGAREADIALOG_H

#include <QDialog>
#include <QLabel>
#include <memory>
#include "controller/maestrocontroller.h"
#include "controller/cueinterpreter.h"
#include "drawingarea/simpledrawingarea.h"

using namespace PixelMaestro;

namespace Ui {
	class SimpleDrawingAreaDialog;
}

class SimpleDrawingAreaDialog : public QDialog {
		Q_OBJECT

	public:
		QLabel* interpreted = nullptr;

		explicit SimpleDrawingAreaDialog(QWidget *parent, MaestroController* maestro_controller);
		~SimpleDrawingAreaDialog();

	private:
		Ui::SimpleDrawingAreaDialog *ui;
		MaestroController* maestro_controller_;
		std::unique_ptr<SimpleDrawingArea> drawing_area_;
};

#endif // SIMPLEDRAWINGAREADIALOG_H
