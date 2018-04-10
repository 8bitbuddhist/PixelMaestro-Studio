/*
 * PaletteDialog - Dialog for creating and editing Palettes.
 */

#include <QColorDialog>
#include "paletteeditdialog.h"
#include "ui_paletteeditdialog.h"
#include "widget/palettecontrolwidget.h"

namespace PixelMaestroStudio {
	/**
	 * Constructor.
	 * Initializes the form and pre-populates fields when editing a Palette.
	 * @param parent Parent widget.
	 * @param target_palette Palette to edit (if applicable).
	 */
	PaletteEditDialog::PaletteEditDialog(PaletteControlWidget* parent, PaletteController::PaletteWrapper* target_palette_wrapper) : QDialog(parent), ui(new Ui::PaletteEditDialog) {
		ui->setupUi(this);

		// If a valid Palette was passed in, pre-populate fields
		this->target_palette_ = target_palette_wrapper;
		if (target_palette_wrapper != nullptr) {
			ui->nameLineEdit->setText(target_palette_wrapper->name);
			ui->numColorsSpinBox->setValue(target_palette_wrapper->palette.get_num_colors());
			ui->typeComboBox->setCurrentIndex((uint8_t)target_palette_wrapper->type);
			ui->reverseCheckBox->setChecked(target_palette_wrapper->mirror);

			this->base_color_ = target_palette_wrapper->base_color;
			parent->set_button_color(ui->baseColorButton, base_color_.r, base_color_.g, base_color_.b);

			this->target_color_ = target_palette_wrapper->target_color;
			parent->set_button_color(ui->targetColorButton, target_color_.r, target_color_.g, target_color_.b);

			on_typeComboBox_currentIndexChanged(ui->typeComboBox->currentIndex());
		}
		else {
			// Hide advanced controls by default
			on_typeComboBox_currentIndexChanged(0);
		}
	}

	/**
	 * Saves changes and closes the form.
	 */
	void PaletteEditDialog::accept() {
		uint8_t num_colors = ui->numColorsSpinBox->value();

		// Only allow the Palette to be created if the name is set
		if (ui->nameLineEdit->text().size() > 0) {
			Colors::RGB colors[num_colors];

			// Handle generation method
			switch ((PaletteController::PaletteType)ui->typeComboBox->currentIndex()) {
				case PaletteController::PaletteType::Blank:
					{
						for (uint8_t i = 0; i < num_colors; i++) {
							colors[i] = {0, 0, 0};
						}
					}
					break;
				case PaletteController::PaletteType::Scaling:
					Colors::generate_scaling_color_array(colors, &base_color_, &target_color_, num_colors, (bool)ui->reverseCheckBox->isChecked());
					break;
				case PaletteController::PaletteType::Random:
					Colors::generate_random_color_array(colors, num_colors);
					break;
			}

			if (target_palette_ != nullptr) {
				// Update the target Palette
				target_palette_->name = ui->nameLineEdit->text();
				target_palette_->palette.set_colors(colors, num_colors);
				target_palette_->type = (PaletteController::PaletteType)ui->typeComboBox->currentIndex();
				target_palette_->base_color = base_color_;
				target_palette_->target_color = target_color_;
				target_palette_->mirror = ui->reverseCheckBox->isChecked();
			}
			else {
				// Add the new Palette
				PaletteControlWidget* parent = static_cast<PaletteControlWidget*>(parentWidget());
				parent->get_palette_controller()->add_palette(ui->nameLineEdit->text(), colors, num_colors, (PaletteController::PaletteType)ui->typeComboBox->currentIndex(), base_color_, target_color_, ui->reverseCheckBox->isChecked());
			}

			QDialog::accept();
		}
		else {
			// If the name is incomplete, highlight the name field.
			ui->nameLabel->setStyleSheet(QString("color: red;"));
		}
	}

	/**
	 * Opens a color picker for the base color.
	 */
	void PaletteEditDialog::on_baseColorButton_clicked() {
		QColor color = QColorDialog::getColor(Qt::white, this, "Select Base Color");
		base_color_ = {(uint8_t)color.red(), (uint8_t)color.green(), (uint8_t)color.blue()};
		PaletteControlWidget* parent = static_cast<PaletteControlWidget*>(parentWidget());
		parent->set_button_color(ui->baseColorButton, base_color_.r, base_color_.g, base_color_.b);
	}

	/**
	 * Opens a color picker for the target color.
	 */
	void PaletteEditDialog::on_targetColorButton_clicked() {
		QColor color = QColorDialog::getColor(Qt::white, this, "Select Target Color");
		target_color_ = {(uint8_t)color.red(), (uint8_t)color.green(), (uint8_t)color.blue()};
		PaletteControlWidget* parent = static_cast<PaletteControlWidget*>(parentWidget());
		parent->set_button_color(ui->targetColorButton, target_color_.r, target_color_.g, target_color_.b);
	}

	/**
	 * Adds or removes certain fields based on the type of Palette selected.
	 * @param index Palette type.
	 */
	void PaletteEditDialog::on_typeComboBox_currentIndexChanged(int index) {
		// SetEnabled doesn't change the appearance for some reason
		ui->baseColorLabel->setVisible(index == 1);
		ui->baseColorButton->setVisible(index == 1);
		ui->targetColorLabel->setVisible(index == 1);
		ui->targetColorButton->setVisible(index == 1);
		ui->reverseCheckBox->setVisible(index == 1);
	}

	PaletteEditDialog::~PaletteEditDialog() {
		delete ui;
	}
}
