#ifndef SECTIONMAPDIALOG_H
#define SECTIONMAPDIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include "controller/devicecontroller.h"
#include "core/maestro.h"

namespace Ui {
	class SectionMapDialog;
}

namespace PixelMaestroStudio {
	class SectionMapDialog : public QDialog {
			Q_OBJECT

		public:
			explicit SectionMapDialog(DeviceController& device, QWidget *parent = nullptr);
			~SectionMapDialog();

		private slots:
			void on_buttonBox_clicked(QAbstractButton *button);

		private:
			DeviceController& device_;
			Ui::SectionMapDialog *ui;

			void initialize();
	};
}

#endif // SECTIONMAPDIALOG_H
