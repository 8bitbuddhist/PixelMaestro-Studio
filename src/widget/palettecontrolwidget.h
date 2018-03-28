#ifndef PALETTECONTROLWIDGET_H
#define PALETTECONTROLWIDGET_H

#include "controller/palettecontroller.h"
#include <QAbstractButton>
#include <QDialog>

namespace Ui {
	class PaletteControlWidget;
}

namespace PixelMaestroStudio {
	class PaletteControlWidget : public QDialog {
			Q_OBJECT

		public:
			explicit PaletteControlWidget(PaletteController* controller, QString initial_palette = "", QWidget *parent = 0);
			~PaletteControlWidget();
			PaletteController* get_palette_controller() const;
			void set_button_color(QPushButton* button, uint8_t red, uint8_t green, uint8_t blue);

		private slots:
			void on_buttonBox_clicked(QAbstractButton *button);
			void on_color_clicked();
			void on_createPaletteButton_clicked();
			void on_paletteComboBox_currentIndexChanged(int index);
			void on_removeButton_clicked();

			void on_editPaletteButton_clicked();

		private:
			Ui::PaletteControlWidget *ui;
			PaletteController::PaletteWrapper* active_palette_ = nullptr;
			PaletteController* palette_controller_ = nullptr;

			void initialize_palettes(QString initial_palette);
	};
}

#endif // PALETTECONTROLWIDGET_H