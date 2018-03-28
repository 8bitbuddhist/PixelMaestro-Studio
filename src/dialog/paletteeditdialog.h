#ifndef PALETTEEDITDIALOG_H
#define PALETTEEDITDIALOG_H

#include <QDialog>
#include "controller/palettecontroller.h"
#include "widget/palettecontrolwidget.h"

namespace Ui {
	class PaletteEditDialog;
}

namespace PixelMaestroStudio {
	class PaletteEditDialog : public QDialog {
			Q_OBJECT

		public:
			explicit PaletteEditDialog(PaletteControlWidget* parent, PaletteController::PaletteWrapper* target_palette = nullptr);
			~PaletteEditDialog();

		private slots:
			void accept();
			void on_baseColorButton_clicked();
			void on_targetColorButton_clicked();
			void on_typeComboBox_currentIndexChanged(int index);

		private:
			Colors::RGB base_color_ = Colors::RGB(0, 0, 0);
			Colors::RGB target_color_ = Colors::RGB(0, 0, 0);

			PaletteControlWidget* parent_ = nullptr;
			PaletteController::PaletteWrapper* target_palette_ = nullptr;
			Ui::PaletteEditDialog *ui;
	};
}

#endif // PALETTEEDITDIALOG_H
