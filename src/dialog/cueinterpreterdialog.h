#ifndef CUEINTERPRETERDIALOG_H
#define CUEINTERPRETERDIALOG_H

#include <QDialog>
#include "controller/cueinterpreter.h"

namespace Ui {
	class CueInterpreterDialog;
}

namespace PixelMaestroStudio {
	class CueInterpreterDialog : public QDialog {
		Q_OBJECT

		public:
			explicit CueInterpreterDialog(QWidget *parent = 0, uint8_t* cuefile_ = nullptr, uint16_t size = 0);
			~CueInterpreterDialog();

		private:
			uint8_t* cuefile_ = nullptr;
			Ui::CueInterpreterDialog *ui;

			CueInterpreter interpreter_;
	};
}

#endif // CUEINTERPRETERDIALOG_H
