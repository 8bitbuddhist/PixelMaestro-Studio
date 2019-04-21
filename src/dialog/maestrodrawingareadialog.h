/*
 * MaestroDrawingAreaDialog - DrawingArea container for Maestro output.
 */

#ifndef MAESTRODRAWINGAREADIALOG_H
#define MAESTRODRAWINGAREADIALOG_H

#include <QDialog>
#include <QLabel>
#include <memory>
#include "controller/maestrocontroller.h"
#include "utility/cueinterpreter.h"
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
			explicit MaestroDrawingAreaDialog(QWidget *parent, MaestroController& maestro_controller);
			MaestroDrawingArea* get_maestro_drawing_area();
			~MaestroDrawingAreaDialog();

		protected:
			bool eventFilter(QObject *watched, QEvent *event);

		private:
			Ui::MaestroDrawingAreaDialog *ui;
			MaestroController& maestro_controller_;
			QSharedPointer<MaestroDrawingArea> drawing_area_;
	};
}

#endif // MAESTRODRAWINGAREADIALOG_H
