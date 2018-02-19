/*
 * MaestroDrawingAreaDialog - DrawingArea container for Maestro output.
 */

#ifndef MAESTRODRAWINGAREADIALOG_H
#define MAESTRODRAWINGAREADIALOG_H

#include <QDialog>
#include <QLabel>
#include <memory>
#include "controller/maestrocontroller.h"
#include "controller/cueinterpreter.h"
#include "drawingarea/maestrodrawingarea.h"

using namespace PixelMaestro;

namespace Ui {
	class MaestroDrawingAreaDialog;
}

namespace PixelMaestroStudio {
	class MaestroController;
	class MaestroDrawingArea;
	class MaestroDrawingAreaDialog : public QDialog {
			Q_OBJECT

		public:
			QLabel* interpreted = nullptr;

			explicit MaestroDrawingAreaDialog(QWidget *parent, MaestroController* maestro_controller);
			~MaestroDrawingAreaDialog();

		protected:
			bool eventFilter(QObject *watched, QEvent *event);

		private:
			Ui::MaestroDrawingAreaDialog *ui;
			MaestroController* maestro_controller_;
			std::unique_ptr<MaestroDrawingArea> drawing_area_;
	};
}

#endif // MAESTRODRAWINGAREADIALOG_H
