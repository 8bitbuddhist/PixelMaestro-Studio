#ifndef CUEINTERPRETERDIALOG_H
#define CUEINTERPRETERDIALOG_H

#include <QDialog>
#include "utility/cueinterpreter.h"

namespace Ui {
	class CueInterpreterDialog;
}

namespace PixelMaestroStudio {
	class CueInterpreterDialog : public QDialog {
		Q_OBJECT

		public:
			explicit CueInterpreterDialog(QWidget *parent = 0, uint8_t* cuefile = nullptr, uint16_t size = 0);
			~CueInterpreterDialog();

		private:
			CueInterpreter interpreter_;

			Ui::CueInterpreterDialog *ui;
	};
}

#endif // CUEINTERPRETERDIALOG_H
