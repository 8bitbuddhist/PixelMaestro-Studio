#include <QAbstractButton>
#include <QColorDialog>
#include <QMessageBox>
#include "dialog/paletteeditdialog.h"
#include "palettecontrolwidget.h"
#include "ui_palettecontrolwidget.h"
#include "utility/uiutility.h"

namespace PixelMaestroStudio {
	PaletteControlWidget::PaletteControlWidget(PaletteController& controller, const QString& initial_palette, QWidget *parent) : QDialog(parent), ui(new Ui::PaletteControlWidget), palette_controller_(controller) {
		ui->setupUi(this);

		initialize_palettes(initial_palette);
	}

	PaletteController& PaletteControlWidget::get_palette_controller() const {
		return this->palette_controller_;
	}

	void PaletteControlWidget::initialize_palettes(const QString& initial_palette) {
		// Initialize palette list
		ui->paletteComboBox->blockSignals(true);
		ui->paletteComboBox->clear();
		for (uint16_t i = 0; i < palette_controller_.get_palettes()->size(); i++) {
			PaletteController::PaletteWrapper& palette = palette_controller_.get_palette(i);
			ui->paletteComboBox->addItem(QIcon(*UIUtility::generate_palette_thumbnail(palette)), palette.name);
		}
		ui->paletteComboBox->blockSignals(false);

		if (initial_palette.length() > 0) {
			ui->paletteComboBox->setCurrentText(initial_palette);
		}
		else {
			ui->paletteComboBox->setCurrentIndex(0);
		}

		// Trigger a Palette redraw just for safe measure
		on_paletteComboBox_currentIndexChanged(ui->paletteComboBox->currentIndex());
	}

	void PaletteControlWidget::on_buttonBox_clicked(QAbstractButton *button) {
		if (button == ui->buttonBox->button(QDialogButtonBox::Reset)) {
			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Reset Palettes", "This will reset all Palettes to their default settings. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
			if (confirm == QMessageBox::Yes) {
				palette_controller_.initialize_palettes();
				initialize_palettes("");
			}
		}
	}

	/// Updates the target color.
	void PaletteControlWidget::on_color_clicked() {
		QPushButton* sender = dynamic_cast<QPushButton*>(QObject::sender());
		Colors::RGB* source_color = &active_palette_->palette.get_colors()[sender->objectName().toInt()];
		QColor new_color = QColorDialog::getColor(QColor(source_color->r, source_color->g, source_color->b), this, "Select Color");
		if (new_color.isValid()) {
			source_color->r = new_color.red();
			source_color->g = new_color.green();
			source_color->b = new_color.blue();
			set_button_color(sender, source_color->r, source_color->g, source_color->b);
		}
	}

	/// Creates a new Palette.
	void PaletteControlWidget::on_createPaletteButton_clicked() {
		PaletteEditDialog dialog(this, nullptr);
		dialog.exec();
		initialize_palettes(palette_controller_.get_palette(palette_controller_.get_palettes()->size() - 1).name);

		// If there's more than one Palette, enable the delete button
		ui->removeButton->setEnabled(palette_controller_.get_palettes()->size() > 1);
	}

	/// Edits the selected Palette.
	void PaletteControlWidget::on_editPaletteButton_clicked() {
		PaletteEditDialog dialog(this, active_palette_);
		dialog.exec();
		initialize_palettes(active_palette_->name);
	}

	/**
	 * Switches the current Palette.
	 * @param index Index of the new Palette.
	 */
	void PaletteControlWidget::on_paletteComboBox_currentIndexChanged(int index) {
		/*
		 * TODO: Find ways to optimize palette switching.
		 *		 Rebuilding QPushButtons takes a long time.
		 */
		active_palette_ = &palette_controller_.get_palette(index);

		// Delete existing color buttons
		QList<QPushButton*> buttons = ui->colorsGroupBox->findChildren<QPushButton*>(QString(), Qt::FindChildOption::FindChildrenRecursively);
		for (QPushButton* button : buttons) {
			disconnect(button, &QPushButton::clicked, this, &PaletteControlWidget::on_color_clicked);
			delete button;
		}

		// Create new buttons and add an event handler that triggers on_color_clicked()
		QLayout* layout = ui->colorsGroupBox->findChild<QLayout*>("colorsLayout");
		for (uint8_t color_index = 0; color_index < active_palette_->palette.get_num_colors(); color_index++) {
			Colors::RGB color = active_palette_->palette.get_colors()[color_index];
			QPushButton* button = new QPushButton();
			button->setVisible(true);
			button->setObjectName(QString::number(color_index));
			button->setToolTip(QString::number(color_index + 1));
			button->setMaximumWidth(40);
			set_button_color(button, color.r, color.g, color.b);

			layout->addWidget(button);
			connect(button, &QPushButton::clicked, this, &PaletteControlWidget::on_color_clicked);
		}
	}

	/// Deletes the current Palette.
	void PaletteControlWidget::on_removeButton_clicked() {
		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Delete Palette", "This will delete the current Palette. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
		if (confirm == QMessageBox::Yes) {
			uint16_t current_index = ui->paletteComboBox->currentIndex();
			palette_controller_.remove_palette(current_index);
			ui->paletteComboBox->removeItem(current_index);
		}

		// If there's only one Palette remaining, disable the delete button
		ui->removeButton->setEnabled(palette_controller_.get_palettes()->size() > 1);
	}

	/**
	 * Changes the color of a button.
	 * @param button Button to change.
	 * @param red Red value.
	 * @param green Green value.
	 * @param blue Blue value.
	 */
	void PaletteControlWidget::set_button_color(QPushButton *button, uint8_t red, uint8_t green, uint8_t blue) {
		button->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(red).arg(green).arg(blue));
	}

	PaletteControlWidget::~PaletteControlWidget() {
		delete ui;
	}
}
