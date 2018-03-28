#include <QColorDialog>
#include "paletteeditdialog.h"
#include "ui_paletteeditdialog.h"
#include "widget/palettecontrolwidget.h"

namespace PixelMaestroStudio {
	PaletteEditDialog::PaletteEditDialog(PaletteControlWidget* parent, PaletteController::PaletteWrapper* target_palette) : QDialog(parent), ui(new Ui::PaletteEditDialog) {
		ui->setupUi(this);

		this->parent_ = parent;

		// If a valid Palette was passed in, pre-populate fields
		this->target_palette_ = target_palette;
		if (target_palette != nullptr) {
			ui->nameLineEdit->setText(target_palette->name);
			ui->numColorsSpinBox->setValue(target_palette->colors.size());
			ui->typeComboBox->setCurrentIndex((uint8_t)target_palette->type);
			ui->reverseCheckBox->setChecked(target_palette->mirror);

			this->base_color_ = target_palette->base_color;
			parent->set_button_color(ui->baseColorButton, base_color_.r, base_color_.g, base_color_.b);

			this->target_color_ = target_palette->target_color;
			parent->set_button_color(ui->targetColorButton, target_color_.r, target_color_.g, target_color_.b);
		}
		else {
			// Hide advanced controls by default
			on_typeComboBox_currentIndexChanged(0);
		}
	}

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
				target_palette_->colors.resize(num_colors);
				for (int i = 0; i < num_colors; i++) {
					target_palette_->colors[i] = colors[i];
				}
				target_palette_->type = (PaletteController::PaletteType)ui->typeComboBox->currentIndex();
				target_palette_->base_color = base_color_;
				target_palette_->target_color = target_color_;
				target_palette_->mirror = ui->reverseCheckBox->isChecked();
			}
			else {
				// Add the new Palette
				this->parent_->get_palette_controller()->add_palette(ui->nameLineEdit->text(), colors, num_colors, (PaletteController::PaletteType)ui->typeComboBox->currentIndex(), base_color_, target_color_, ui->reverseCheckBox->isChecked());
			}

			QDialog::accept();
		}
		else {
			// Highlight name label
			ui->nameLabel->setStyleSheet(QString("color: red;"));
		}
	}

	void PaletteEditDialog::on_baseColorButton_clicked() {
		QColor color = QColorDialog::getColor(Qt::white, this, "Select Base Color");
		base_color_ = {color.red(), color.green(), color.blue()};
		this->parent_->set_button_color(ui->baseColorButton, color.red(), color.green(), color.blue());
	}

	void PaletteEditDialog::on_targetColorButton_clicked() {
		QColor color = QColorDialog::getColor(Qt::white, this, "Select Target Color");
		target_color_ = {color.red(), color.green(), color.blue()};
		this->parent_->set_button_color(ui->targetColorButton, color.red(), color.green(), color.blue());
	}

	void PaletteEditDialog::on_typeComboBox_currentIndexChanged(int index) {
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
