#ifndef SECTIONMAPDIALOG_H
#define SECTIONMAPDIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include "controller/serialdevicecontroller.h"
#include "core/maestro.h"

namespace Ui {
	class SectionMapDialog;
}

namespace PixelMaestroStudio {
	class SectionMapDialog : public QDialog {
			Q_OBJECT

		public:
			explicit SectionMapDialog(SerialDeviceController& device, QWidget *parent = nullptr);
			~SectionMapDialog();

		private slots:
			void on_buttonBox_clicked(QAbstractButton *button);

		private:
			SerialDeviceController& device_;
			Ui::SectionMapDialog *ui;

			void initialize();
	};
}

#endif // SECTIONMAPDIALOG_H
