#ifndef CUEINTERPRETERDIALOG_H
#define CUEINTERPRETERDIALOG_H

#include <QDialog>
#include "model/cuemodel.h"

namespace Ui {
	class CueInterpreterDialog;
}

namespace PixelMaestroStudio {
	class CueInterpreterDialog : public QDialog {
		Q_OBJECT

		public:
			explicit CueInterpreterDialog(QWidget *parent = 0, uint8_t* cuefile = nullptr, uint32_t size = 0);
			~CueInterpreterDialog();

		private slots:
			void on_copyButton_clicked();
			void on_closeButton_clicked();

		private:
			Ui::CueInterpreterDialog *ui;
			CueModel model_;
	};
}

#endif // CUEINTERPRETERDIALOG_H
