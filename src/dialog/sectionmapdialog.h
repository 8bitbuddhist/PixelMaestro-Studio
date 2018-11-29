#ifndef SECTIONMAPDIALOG_H
#define SECTIONMAPDIALOG_H

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
			explicit SectionMapDialog(SerialDeviceController* device, QWidget *parent = nullptr);
			~SectionMapDialog();

		private slots:
			void on_addSectionButton_clicked();

			void on_removeSectionButton_clicked();

			void on_buttonBox_accepted();

		private:
			SectionMapModel* model_ = nullptr;
			Ui::SectionMapDialog *ui;
	};
}

#endif // SECTIONMAPDIALOG_H
