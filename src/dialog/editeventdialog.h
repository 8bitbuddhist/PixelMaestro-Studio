#ifndef EDITEVENTDIALOG_H
#define EDITEVENTDIALOG_H

#include <QDialog>
#include "cue/event.h"

namespace Ui {
	class EditEventDialog;
}

using namespace	PixelMaestro;

namespace PixelMaestroStudio {
	class EditEventDialog : public QDialog {
			Q_OBJECT

		public:
			explicit EditEventDialog(Event* event, QWidget *parent = nullptr);
			~EditEventDialog();

		private slots:
			void on_buttonBox_accepted();

		private:
			Event* event_ = nullptr;
			Ui::EditEventDialog *ui;
	};
}

#endif // EDITEVENTDIALOG_H
