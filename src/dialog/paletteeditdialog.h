/*
 * PaletteDialog - Dialog for creating and editing Palettes.
 */

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
			explicit PaletteEditDialog(PaletteControlWidget* parent, PaletteController::PaletteWrapper* target_palette_wrapper = nullptr);
			~PaletteEditDialog();

		private slots:
			void accept();
			void on_baseColorButton_clicked();
			void on_targetColorButton_clicked();
			void on_typeComboBox_currentIndexChanged(int index);

			void on_numColorsSpinBox_valueChanged(int arg1);

			void on_reverseCheckBox_stateChanged(int arg1);

		private:
			/// The starting color when scaling.
			Colors::RGB base_color_ = Colors::RGB(0, 0, 0);

			/// Flags whether to rebuild the color array when saving a Palette
			bool colors_changed_ = false;

			/// The final color when scaling.
			Colors::RGB target_color_ = Colors::RGB(0, 0, 0);

			/// The Palette that we're currently editing.
			PaletteController::PaletteWrapper* target_palette_ = nullptr;

			Ui::PaletteEditDialog *ui;
	};
}

#endif // PALETTEEDITDIALOG_H
